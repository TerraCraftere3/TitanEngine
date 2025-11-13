#include "Renderer3D.h"
#include "RenderCommand.h"
#include "Shader.h"
#include "ShaderStorageBuffer.h"
#include "Titan/PCH.h"
#include "Titan/Scene/Assets.h"
#include "UniformBuffer.h"
#include "VertexArray.h"

namespace Titan
{

    struct Vertex
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoord;
        int EntityID;
        int MaterialIndex = 0;
    };

    struct Renderer3DData
    {
        static const uint32_t MaxVertices = 100'000;
        static const uint32_t MaxMaterials = 1000;

        Ref<VertexArray> VertexArray;
        Ref<VertexBuffer> VertexBuffer;
        Vertex* VertexBufferBase = nullptr;
        Vertex* VertexBufferPtr = nullptr;
        uint32_t VertexCount = 0;

        struct CameraData
        {
            glm::mat4 ViewProjection;
            glm::vec3 ViewPosition;
            bool HasDirectionalLight;
            glm::vec3 LightDirection;
            float Padding; // Padding to align to 16 bytes
        };
        CameraData CamBuffer;

        Ref<Shader> Shader;
        Ref<UniformBuffer> CameraUniformBuffer;
        Ref<ShaderStorageBuffer> MaterialStorageBuffer;

        // Material management
        std::vector<Material3D> Materials;
        std::unordered_map<size_t, uint32_t> MaterialIndexMap; // Hash -> Index
        uint32_t CurrentMaterialIndex = 0;

        Renderer3D::Statistics Stats;
    };

    static Renderer3DData s_3DData;
    static bool s_IsRendering = false;

    // Helper function to hash materials
    static size_t HashMaterial(const Material3D& mat)
    {
        size_t hash = 0;
        const uint8_t* data = reinterpret_cast<const uint8_t*>(&mat);
        for (size_t i = 0; i < sizeof(Material3D); ++i)
        {
            hash = hash * 31 + data[i];
        }
        return hash;
    }

    void Renderer3D::Init()
    {
        TI_PROFILE_FUNCTION();

        s_3DData.VertexBufferBase = new Vertex[s_3DData.MaxVertices];
        s_3DData.VertexBufferPtr = s_3DData.VertexBufferBase;
        s_3DData.VertexCount = 0;

        s_3DData.VertexArray = VertexArray::Create();
        s_3DData.VertexBuffer = VertexBuffer::Create(s_3DData.MaxVertices * sizeof(Vertex));

        // clang-format off
        s_3DData.VertexBuffer->SetLayout({
            {ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float3, "a_Normal"},
            {ShaderDataType::Float2, "a_TexCoord"},
            {ShaderDataType::Int,    "a_EntityID"},
            {ShaderDataType::Int,    "a_MaterialIndex"}
        });
        // clang-format on

        s_3DData.VertexArray->AddVertexBuffer(s_3DData.VertexBuffer);

        s_3DData.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer3DData::CameraData), 0);
        s_3DData.MaterialStorageBuffer =
            ShaderStorageBuffer::Create(sizeof(Material3D) * s_3DData.MaxMaterials, 1); // 256 Materials Max
        s_3DData.Shader = Shader::Create("assets/shader/Mesh.slang");

        // Reserve space for materials
        s_3DData.Materials.reserve(s_3DData.MaxMaterials);
    }

    void Renderer3D::Shutdown()
    {
        TI_PROFILE_FUNCTION();

        delete[] s_3DData.VertexBufferBase;
        s_3DData.VertexBufferBase = nullptr;

        s_3DData.VertexArray.reset();
        s_3DData.VertexBuffer.reset();
        s_3DData.Shader.reset();
        s_3DData.CameraUniformBuffer.reset();
        s_3DData.MaterialStorageBuffer.reset();

        s_3DData.Materials.clear();
        s_3DData.MaterialIndexMap.clear();
    }

    void Renderer3D::BeginScene(const glm::mat4& viewProjectionMatrix, const glm::vec3& viewPosition,
                                bool hasDirectionalLight, glm::vec3 lightDirection)
    {
        TI_PROFILE_FUNCTION();
        TI_CORE_ASSERT(!s_IsRendering, "Forgot to call Renderer3D::EndScene()?");
        TI_CORE_ASSERT(s_3DData.VertexBufferBase != nullptr, "Renderer3D not initialized!");

        s_3DData.CamBuffer.ViewProjection = viewProjectionMatrix;
        s_3DData.CamBuffer.HasDirectionalLight = hasDirectionalLight;
        s_3DData.CamBuffer.LightDirection = lightDirection;
        s_3DData.CameraUniformBuffer->SetData(&s_3DData.CamBuffer, sizeof(Renderer3DData::CameraData));

        s_3DData.Shader->Bind();

        StartBatch();
        s_IsRendering = true;
    }

    void Renderer3D::EndScene()
    {
        TI_PROFILE_FUNCTION();
        TI_CORE_ASSERT(s_IsRendering, "Called Renderer3D::EndScene() without BeginScene()");

        Flush();
        s_IsRendering = false;

        // Clear materials for next frame
        s_3DData.Materials.clear();
        s_3DData.MaterialIndexMap.clear();
        s_3DData.CurrentMaterialIndex = 0;
    }

    void Renderer3D::StartBatch()
    {
        s_3DData.VertexCount = 0;
        s_3DData.VertexBufferPtr = s_3DData.VertexBufferBase;
    }

    void Renderer3D::Flush()
    {
        TI_PROFILE_FUNCTION();

        if (s_3DData.VertexCount == 0)
            return;

        // Upload materials to GPU
        if (!s_3DData.Materials.empty())
        {
            s_3DData.MaterialStorageBuffer->SetData(s_3DData.Materials.data(),
                                                    s_3DData.Materials.size() * sizeof(Material3D));
        }

        // Upload vertex data
        uint32_t dataSize = (uint32_t)((uint8_t*)s_3DData.VertexBufferPtr - (uint8_t*)s_3DData.VertexBufferBase);
        s_3DData.VertexBuffer->SetData(s_3DData.VertexBufferBase, dataSize);
        s_3DData.Shader->Bind();
        s_3DData.MaterialStorageBuffer->Bind();

        RenderCommand::DrawArrays(s_3DData.VertexArray, s_3DData.VertexCount);

        s_3DData.Stats.DrawCalls++;
        s_3DData.Stats.VertexCount += s_3DData.VertexCount;
    }

    void Renderer3D::FlushAndReset()
    {
        TI_PROFILE_FUNCTION();

        Flush();
        StartBatch();
    }

    static uint32_t GetOrAddMaterial(const Material3D& mat)
    {
        size_t hash = HashMaterial(mat);

        auto it = s_3DData.MaterialIndexMap.find(hash);
        if (it != s_3DData.MaterialIndexMap.end())
        {
            return it->second;
        }

        // Add new material
        uint32_t newIndex = s_3DData.CurrentMaterialIndex++;

        if (newIndex >= s_3DData.MaxMaterials)
        {
            TI_CORE_WARN("Material limit reached! Using default material.");
            return 0;
        }

        s_3DData.Materials.push_back(mat);
        s_3DData.MaterialIndexMap[hash] = newIndex;

        return newIndex;
    }

    void Renderer3D::DrawMesh(const Ref<Mesh>& mesh, const Material3D& mat, const glm::mat4& transform, int entityID)
    {
        TI_PROFILE_FUNCTION();
        TI_CORE_ASSERT(s_IsRendering, "Must call BeginScene() before DrawMesh()");

        if (!mesh)
            return;

        const auto& positions = mesh->GetPositions();
        const auto& normals = mesh->GetNormals();
        const auto& texCoords = mesh->GetTexCoords();

        uint32_t totalVertexCount = (uint32_t)positions.size();

        // Get or add material index
        uint32_t materialIndex = GetOrAddMaterial(mat);

        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));

        uint32_t vertexOffset = 0;
        while (vertexOffset < totalVertexCount)
        {
            uint32_t availableSpace = s_3DData.MaxVertices - s_3DData.VertexCount;
            uint32_t remainingVertices = totalVertexCount - vertexOffset;

            uint32_t verticesThisChunk = min(availableSpace, remainingVertices);
            verticesThisChunk = (verticesThisChunk / 3) * 3;

            if (verticesThisChunk == 0)
            {
                FlushAndReset();
                availableSpace = s_3DData.MaxVertices;
                verticesThisChunk = min(availableSpace, remainingVertices);
                verticesThisChunk = (verticesThisChunk / 3) * 3;

                TI_CORE_ASSERT(verticesThisChunk >= 3, "MaxVertices too small to render triangles!");
            }

            for (uint32_t i = 0; i < verticesThisChunk; ++i)
            {
                uint32_t vertexIndex = vertexOffset + i;

                glm::vec4 transformedPos = transform * glm::vec4(positions[vertexIndex], 1.0f);
                s_3DData.VertexBufferPtr->Position = glm::vec3(transformedPos);

                s_3DData.VertexBufferPtr->Normal = glm::normalize(normalMatrix * normals[vertexIndex]);

                s_3DData.VertexBufferPtr->TexCoord = texCoords[vertexIndex];
                s_3DData.VertexBufferPtr->EntityID = entityID;
                s_3DData.VertexBufferPtr->MaterialIndex = materialIndex;

                s_3DData.VertexBufferPtr++;
            }

            s_3DData.VertexCount += verticesThisChunk;
            vertexOffset += verticesThisChunk;
        }

        s_3DData.Stats.MeshCount++;
    }

    Renderer3D::Statistics Renderer3D::GetStats()
    {
        return s_3DData.Stats;
    }

    void Renderer3D::ResetStats()
    {
        memset(&s_3DData.Stats, 0, sizeof(Statistics));
    }

} // namespace Titan