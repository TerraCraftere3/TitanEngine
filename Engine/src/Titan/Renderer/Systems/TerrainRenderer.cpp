#include "TerrainRenderer.h"

#include "Titan/Renderer/RHI/Shader.h"
#include "Titan/Renderer/RHI/UniformBuffer.h"
#include "Titan/Renderer/RHI/VertexArray.h"
#include "Titan/Renderer/RenderCommand.h"
#include "Titan/Renderer/Utils/QuadMeshGenerator.h"
#include "Titan/Scene/Assets.h"
#include "Titan/Utils/PlatformUtils.h"

namespace Titan
{
    struct UniformBufferObject
    {
        glm::mat4 ViewProjection;
        glm::mat4 Transform;
        uint32_t EntityID;
        float _pad[3]; // Padding to ensure 16-byte alignment
    };

    struct TerrainRendererData
    {
        Ref<Shader> Shader;
        Ref<PipelineState> Pipeline;
        Ref<UniformBuffer> UniformBuffer;
        UniformBufferObject UBO;
        Ref<VertexArray> QuadVAO;
        Ref<VertexBuffer> QuadVertexBuffer;
        Ref<IndexBuffer> QuadIndexBuffer;
    };

    // JUST A REFERENCE STRUCT, NOT TO BE CONFUSED WITH THE QUADMESHVERTEX STRUCT IN QuadMeshGenerator
    /*struct Vertex
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec3 Tangent;
        glm::vec2 TexCoord;
    };*/

    static TerrainRendererData s_TerrainData;

    void TerrainRenderer::Init()
    {
        auto shaderpath = Filesystem::GetExecutableDirectory() / "resources" / "shader" / "RendererTerrain.slang";
        s_TerrainData.Shader = Assets::Load<Shader>(shaderpath.string());
#ifdef TI_BUILD_DEBUG
        Assets::AttachHotreloader<Shader>(shaderpath.string());
#endif

        s_TerrainData.UniformBuffer = UniformBuffer::Create(sizeof(UniformBufferObject));

        s_TerrainData.QuadVAO = VertexArray::Create();

        constexpr uint32_t quadVertices = 128;
        auto quadMesh = QuadMeshGenerator::GenerateQuad(quadVertices, quadVertices, 1.0f, 1.0f);
        s_TerrainData.QuadVertexBuffer = VertexBuffer::Create(quadMesh.Vertices.size() * sizeof(QuadMeshVertex));
        s_TerrainData.QuadVertexBuffer->SetLayout({{ShaderDataType::Float3, "a_Position"},
                                                   {ShaderDataType::Float3, "a_Normal"},
                                                   {ShaderDataType::Float3, "a_Tangent"},
                                                   {ShaderDataType::Float2, "a_TexCoord"}});

        s_TerrainData.QuadVertexBuffer->SetData(quadMesh.Vertices.data(),
                                                quadMesh.Vertices.size() * sizeof(QuadMeshVertex));
        s_TerrainData.QuadVAO->AddVertexBuffer(s_TerrainData.QuadVertexBuffer);

        s_TerrainData.QuadIndexBuffer = IndexBuffer::Create(quadMesh.Indices.data(), quadMesh.Indices.size());
        s_TerrainData.QuadVAO->SetIndexBuffer(s_TerrainData.QuadIndexBuffer);

        s_TerrainData.Pipeline = PipelineState::Create();
        s_TerrainData.Pipeline->SetShader(s_TerrainData.Shader);
        s_TerrainData.Pipeline->SetUniformBuffer(s_TerrainData.UniformBuffer, 0);
    }

    void TerrainRenderer::Shutdown()
    {
        // Cleanup terrain shader resources here
    }

    void TerrainRenderer::BeginScene(const glm::mat4& viewTransform, PolygonMode polygonMode)
    {
        s_TerrainData.Pipeline->SetPolygonMode(polygonMode);
        s_TerrainData.UBO.ViewProjection = viewTransform;
    }

    void TerrainRenderer::EndScene() {}

    void TerrainRenderer::DrawTerrain(const Ref<Texture2D>& heightMap, const glm::mat4& transform, int entityID)
    {
        s_TerrainData.UBO.Transform = transform;
        s_TerrainData.UBO.EntityID = static_cast<uint32_t>(entityID);
        s_TerrainData.UniformBuffer->SetData(&s_TerrainData.UBO, sizeof(UniformBufferObject));

        s_TerrainData.Pipeline->SetTexture(heightMap, 0);
        s_TerrainData.Pipeline->SetVertexArray(s_TerrainData.QuadVAO);
        s_TerrainData.Pipeline->Bind();
        RenderCommand::DrawIndexed(s_TerrainData.QuadIndexBuffer->GetCount());
    }
} // namespace Titan