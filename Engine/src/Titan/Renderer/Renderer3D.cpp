#include "Renderer3D.h"
#include "RenderCommand.h"
#include "Renderer2D.h"
#include "Shader.h"
#include "ShaderStorageBuffer.h"
#include "Titan/PCH.h"
#include "Titan/Scene/Assets.h"
#include "UniformBuffer.h"
#include "VertexArray.h"

namespace Titan
{
    glm::uvec2 HandleToVec2(uint64_t handle)
    {
        uint32_t low = static_cast<uint32_t>(handle & 0xFFFFFFFF);          // lower 32 bits
        uint32_t high = static_cast<uint32_t>((handle >> 32) & 0xFFFFFFFF); // upper 32 bits
        return glm::uvec2(low, high);
    }

    struct Textures
    {
        Ref<Texture2D> DefaultAlbedo;
        Ref<Texture2D> DefaultMetallic;
        Ref<Texture2D> DefaultRoughness;
        Ref<Texture2D> DefaultNormal;
        Ref<Texture2D> DefaultAO;
    };

    static Textures s_Textures;

    struct Vertex
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec3 Tangent;
        glm::vec2 TexCoord;
        int EntityID;
        int MaterialIndex = 0;
    };

    struct alignas(16) GPUMaterial
    {
        glm::vec4 AlbedoColor; // 16 bytes

        glm::uvec2 AlbedoTextureIndex; // 8 bytes
        float Metallic;                // 4 bytes
        float Padding0;                // 4 bytes, align next member

        glm::uvec2 MetallicTextureIndex; // 8 bytes
        glm::uvec2 AOTextureIndex;       // 8 bytes;

        glm::uvec2 RoughnessTextureIndex; // 8 bytes
        glm::uvec2 NormalTextureIndex;    // 8 bytes

        float Padding2[2]; // 8 bytes, round struct to 16-byte multiple

        GPUMaterial() = default;

        explicit GPUMaterial(const Material3D& mat)
        {
            AlbedoColor = mat.AlbedoColor;
            if (mat.AlbedoTexture)
                AlbedoTextureIndex = HandleToVec2(mat.AlbedoTexture->GetBindlessHandle());
            else
                AlbedoTextureIndex = HandleToVec2(s_Textures.DefaultAlbedo->GetBindlessHandle());

            if (mat.MetallicTexture)
                MetallicTextureIndex = HandleToVec2(mat.MetallicTexture->GetBindlessHandle());
            else
                MetallicTextureIndex = HandleToVec2(s_Textures.DefaultMetallic->GetBindlessHandle());

            if (mat.RoughnessTexture)
                RoughnessTextureIndex = HandleToVec2(mat.RoughnessTexture->GetBindlessHandle());
            else
                RoughnessTextureIndex = HandleToVec2(s_Textures.DefaultRoughness->GetBindlessHandle());

            if (mat.NormalTexture)
                NormalTextureIndex = HandleToVec2(mat.NormalTexture->GetBindlessHandle());
            else
                NormalTextureIndex = HandleToVec2(s_Textures.DefaultNormal->GetBindlessHandle());

            if (mat.AOTexture)
                AOTextureIndex = HandleToVec2(mat.AOTexture->GetBindlessHandle());
            else
                AOTextureIndex = HandleToVec2(s_Textures.DefaultAO->GetBindlessHandle());
        }
    };

    struct Renderer3DData
    {
        static const uint32_t MaxVertices = 100'000;
        static const uint32_t MaxMaterials = 1000;
        static const uint32_t MaxTextures = 1024;

        Ref<VertexArray> VertexArray;
        Ref<VertexBuffer> VertexBuffer;
        Vertex* VertexBufferBase = nullptr;
        Vertex* VertexBufferPtr = nullptr;
        uint32_t VertexCount = 0;

        struct CameraData
        {
            glm::mat4 ViewProjection;
        };
        CameraData CamBuffer;

        Ref<Shader> Shader;
        Ref<UniformBuffer> CameraUniformBuffer;
        Ref<ShaderStorageBuffer> MaterialStorageBuffer;
        Ref<ShaderStorageBuffer> TextureStorageBuffer;

        std::vector<GPUMaterial> GPUMaterials;
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
            {ShaderDataType::Float3, "a_Tangent"},
            {ShaderDataType::Float2, "a_TexCoord"},
            {ShaderDataType::Int,    "a_EntityID"},
            {ShaderDataType::Int,    "a_MaterialIndex"}
        });
        // clang-format on

        s_3DData.VertexArray->AddVertexBuffer(s_3DData.VertexBuffer);

        s_3DData.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer3DData::CameraData), 0);
        s_3DData.MaterialStorageBuffer = ShaderStorageBuffer::Create(sizeof(GPUMaterial) * s_3DData.MaxMaterials, 1);
        s_3DData.TextureStorageBuffer = ShaderStorageBuffer::Create(sizeof(glm::ivec2) * s_3DData.MaxMaterials, 2);
        s_3DData.Shader = Shader::Create("assets/shader/RendererGeometry.slang");

        // Reserve space for GPU materials
        s_3DData.GPUMaterials.reserve(s_3DData.MaxMaterials);

        // Textures
        s_Textures.DefaultAlbedo = Texture2D::Create(1, 1);
        {
            uint32_t data = 0xffffffff; // white
            s_Textures.DefaultAlbedo->SetData(&data, sizeof(uint32_t));
        }

        s_Textures.DefaultMetallic = Texture2D::Create(1, 1);
        {
            uint32_t data = 0x00000000; // black
            s_Textures.DefaultMetallic->SetData(&data, sizeof(uint32_t));
        }

        s_Textures.DefaultRoughness = Texture2D::Create(1, 1);
        {
            uint32_t data = 0xffffffff; // white
            s_Textures.DefaultRoughness->SetData(&data, sizeof(uint32_t));
        }

        s_Textures.DefaultNormal = Texture2D::Create(1, 1);
        {
            uint32_t data = (255u << 24) | // A
                            (255u << 16) | // B = 255
                            (128u << 8) |  // G = 128
                            (128u);        // R = 128
            s_Textures.DefaultNormal->SetData(&data, sizeof(uint32_t));
        }

        s_Textures.DefaultAO = Texture2D::Create(1, 1);
        {
            uint32_t data = 0xffffffff; // white
            s_Textures.DefaultAO->SetData(&data, sizeof(uint32_t));
        }
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

        s_3DData.GPUMaterials.clear();
        s_3DData.MaterialIndexMap.clear();

        s_Textures = {};
    }

    void Renderer3D::BeginScene(const glm::mat4& viewProjectionMatrix)
    {
        TI_PROFILE_FUNCTION();
        TI_CORE_ASSERT(!s_IsRendering, "Forgot to call Renderer3D::EndScene()?");
        TI_CORE_ASSERT(s_3DData.VertexBufferBase != nullptr, "Renderer3D not initialized!");

        s_3DData.CamBuffer.ViewProjection = viewProjectionMatrix;
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
        s_3DData.GPUMaterials.clear();
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

        // Upload GPU materials to GPU
        if (!s_3DData.GPUMaterials.empty())
        {
            s_3DData.MaterialStorageBuffer->SetData(s_3DData.GPUMaterials.data(),
                                                    s_3DData.GPUMaterials.size() * sizeof(GPUMaterial));
        }

        // Upload vertex data
        uint32_t dataSize = (uint32_t)((uint8_t*)s_3DData.VertexBufferPtr - (uint8_t*)s_3DData.VertexBufferBase);
        s_3DData.VertexBuffer->SetData(s_3DData.VertexBufferBase, dataSize);
        s_3DData.Shader->Bind();
        s_3DData.MaterialStorageBuffer->Bind();
        s_3DData.CameraUniformBuffer->Bind();

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

        uint32_t newIndex = static_cast<uint32_t>(s_3DData.GPUMaterials.size());

        if (newIndex >= s_3DData.MaxMaterials)
        {
            TI_CORE_ERROR("Material limit reached! Max: {}", s_3DData.MaxMaterials);
            return 0;
        }

        GPUMaterial gpuMat(mat);

        s_3DData.GPUMaterials.push_back(gpuMat);
        s_3DData.MaterialIndexMap[hash] = newIndex;

        size_t materialDataSize = s_3DData.GPUMaterials.size() * sizeof(GPUMaterial);

        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&gpuMat);
        std::string hexDump;
        for (size_t i = 0; i < sizeof(GPUMaterial); ++i)
        {
            char buf[4];
            snprintf(buf, sizeof(buf), "%02X ", bytes[i]);
            hexDump += buf;
        }

        s_3DData.MaterialStorageBuffer->SetData(s_3DData.GPUMaterials.data(), materialDataSize);

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
        const auto& tangents = mesh->GetTangents();
        const auto& texCoords = mesh->GetTexCoords();

        uint32_t totalVertexCount = (uint32_t)positions.size();

        // Get or add material index - Material3D is automatically converted to GPUMaterial
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
                s_3DData.VertexBufferPtr->Tangent = glm::normalize(glm::mat3(transform) * tangents[vertexIndex]);

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