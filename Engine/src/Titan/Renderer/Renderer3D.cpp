#include "Renderer3D.h"
#include "RenderCommand.h"
#include "Shader.h"
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
    };

    struct Renderer3DData
    {
        static const uint32_t MaxVertices = 100'000;

        Ref<VertexArray> VertexArray;
        Ref<VertexBuffer> VertexBuffer;
        Vertex* VertexBufferBase = nullptr;
        Vertex* VertexBufferPtr = nullptr;
        uint32_t VertexCount = 0;

        Ref<Shader> Shader;
        Ref<UniformBuffer> CameraUniformBuffer;

        glm::mat4 ViewProjectionMatrix;

        Renderer3D::Statistics Stats;
    };

    static Renderer3DData s_3DData;
    static bool s_IsRendering = false;

    void Renderer3D::Init()
    {
        TI_PROFILE_FUNCTION();

        // Allocate CPU buffer FIRST before anything else
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
            {ShaderDataType::Int,    "a_EntityID"}
        });
        // clang-format on

        s_3DData.VertexArray->AddVertexBuffer(s_3DData.VertexBuffer);

        // Create camera uniform buffer
        s_3DData.CameraUniformBuffer = UniformBuffer::Create(sizeof(glm::mat4), 0);

        // Load shader
        s_3DData.Shader = Shader::Create("assets/shader/Mesh.slang");
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
    }

    void Renderer3D::BeginScene(const EditorCamera& camera)
    {
        BeginScene(camera.GetViewProjection());
    }

    void Renderer3D::BeginScene(const Camera& camera, const glm::mat4& transform)
    {
        BeginScene(camera.GetProjection() * glm::inverse(transform));
    }

    void Renderer3D::BeginScene(const glm::mat4& viewProjectionMatrix)
    {
        TI_PROFILE_FUNCTION();
        TI_CORE_ASSERT(!s_IsRendering, "Forgot to call Renderer3D::EndScene()?");
        TI_CORE_ASSERT(s_3DData.VertexBufferBase != nullptr, "Renderer3D not initialized!");

        s_3DData.ViewProjectionMatrix = viewProjectionMatrix;
        s_3DData.CameraUniformBuffer->SetData(&s_3DData.ViewProjectionMatrix, sizeof(glm::mat4));

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

        uint32_t dataSize = (uint32_t)((uint8_t*)s_3DData.VertexBufferPtr - (uint8_t*)s_3DData.VertexBufferBase);
        s_3DData.VertexBuffer->SetData(s_3DData.VertexBufferBase, dataSize);

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

    void Renderer3D::DrawMesh(const Ref<Mesh>& mesh, const glm::mat4& transform, int entityID)
    {
        TI_PROFILE_FUNCTION();
        TI_CORE_ASSERT(s_IsRendering, "Must call BeginScene() before DrawMesh()");

        if (!mesh)
            return;

        const auto& positions = mesh->GetPositions();
        const auto& normals = mesh->GetNormals();
        const auto& texCoords = mesh->GetTexCoords();

        uint32_t totalVertexCount = (uint32_t)positions.size();

        // Pre-compute transformation matrices once
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(transform)));

        // Process mesh in triangle-aligned chunks
        uint32_t vertexOffset = 0;
        while (vertexOffset < totalVertexCount)
        {
            // Calculate how many vertices we can fit in this batch
            uint32_t availableSpace = s_3DData.MaxVertices - s_3DData.VertexCount;
            uint32_t remainingVertices = totalVertexCount - vertexOffset;

            // Round down to nearest multiple of 3 to keep triangles intact
            uint32_t verticesThisChunk = min(availableSpace, remainingVertices);
            verticesThisChunk = (verticesThisChunk / 3) * 3;

            // If we can't fit even one triangle, flush and reset
            if (verticesThisChunk == 0)
            {
                FlushAndReset();
                availableSpace = s_3DData.MaxVertices;
                verticesThisChunk = min(availableSpace, remainingVertices);
                verticesThisChunk = (verticesThisChunk / 3) * 3;

                // Safety check: if MaxVertices can't hold a single triangle
                TI_CORE_ASSERT(verticesThisChunk >= 3, "MaxVertices too small to render triangles!");
            }

            // Transform and add vertices for this chunk
            for (uint32_t i = 0; i < verticesThisChunk; ++i)
            {
                uint32_t vertexIndex = vertexOffset + i;

                glm::vec4 transformedPos = transform * glm::vec4(positions[vertexIndex], 1.0f);
                s_3DData.VertexBufferPtr->Position = glm::vec3(transformedPos);

                // Transform normal (use inverse transpose for correct normal transformation)
                s_3DData.VertexBufferPtr->Normal = glm::normalize(normalMatrix * normals[vertexIndex]);

                s_3DData.VertexBufferPtr->TexCoord = texCoords[vertexIndex];
                s_3DData.VertexBufferPtr->EntityID = entityID;

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