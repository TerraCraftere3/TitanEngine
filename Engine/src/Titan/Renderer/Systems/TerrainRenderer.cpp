#include "TerrainRenderer.h"

#include "Titan/Renderer/RHI/PipelineState.h"
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
        float _pad1[3];
        glm::vec4 AlbedoColor;
        glm::vec2 UVRepeat;
        float _pad2[2];
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

    struct Textures
    {
        Ref<Texture2D> DefaultAlbedo;
        Ref<Texture2D> DefaultEmission;
        Ref<Texture2D> DefaultMetallic;
        Ref<Texture2D> DefaultRoughness;
        Ref<Texture2D> DefaultNormal;
        Ref<Texture2D> DefaultAO;
    };

    static Textures s_Textures;
    static TerrainRendererData s_TerrainData;

    // JUST A REFERENCE STRUCT, NOT TO BE CONFUSED WITH THE QUADMESHVERTEX STRUCT IN QuadMeshGenerator
    /*struct Vertex
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec3 Tangent;
        glm::vec2 TexCoord;
    };*/

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

        s_Textures.DefaultAlbedo = Texture2D::Create(1, 1);
        uint32_t white = 0xffffffff;
        s_Textures.DefaultAlbedo->SetData(&white, sizeof(uint32_t));

        s_Textures.DefaultEmission = Texture2D::Create(1, 1);
        uint32_t black = 0x00000000;
        s_Textures.DefaultEmission->SetData(&black, sizeof(uint32_t));

        s_Textures.DefaultMetallic = Texture2D::Create(1, 1);
        s_Textures.DefaultMetallic->SetData(&black, sizeof(uint32_t));

        s_Textures.DefaultRoughness = Texture2D::Create(1, 1);
        s_Textures.DefaultRoughness->SetData(&white, sizeof(uint32_t));

        s_Textures.DefaultNormal = Texture2D::Create(1, 1);
        uint32_t normalData = (255u << 24) | (255u << 16) | (128u << 8) | 128u;
        s_Textures.DefaultNormal->SetData(&normalData, sizeof(uint32_t));

        s_Textures.DefaultAO = Texture2D::Create(1, 1);
        s_Textures.DefaultAO->SetData(&white, sizeof(uint32_t));
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

    void TerrainRenderer::DrawTerrain(const Ref<Texture2D>& heightMap, const Ref<Material3D>& material,
                                      const glm::mat4& transform, int entityID)
    {
        s_TerrainData.UBO.Transform = transform;
        s_TerrainData.UBO.EntityID = static_cast<uint32_t>(entityID);
        s_TerrainData.UBO.AlbedoColor = material ? material->AlbedoColor : glm::vec4(1.0f);
        s_TerrainData.UBO.UVRepeat = material ? material->UVRepeat : glm::vec2(1.0f);
        s_TerrainData.UniformBuffer->SetData(&s_TerrainData.UBO, sizeof(UniformBufferObject));

        s_TerrainData.Pipeline->SetTexture(heightMap, 0);
        s_TerrainData.Pipeline->SetVertexArray(s_TerrainData.QuadVAO);

        // clang-format off
        auto albedo    = material && material->AlbedoTexture    ? material->AlbedoTexture    : s_Textures.DefaultAlbedo;
        auto emission  = material && material->EmissionTexture  ? material->EmissionTexture  : s_Textures.DefaultEmission;
        auto metallic  = material && material->MetallicTexture  ? material->MetallicTexture  : s_Textures.DefaultMetallic;
        auto roughness = material && material->RoughnessTexture ? material->RoughnessTexture : s_Textures.DefaultRoughness;
        auto normal    = material && material->NormalTexture    ? material->NormalTexture    : s_Textures.DefaultNormal;
        auto ao        = material && material->AOTexture        ? material->AOTexture        : s_Textures.DefaultAO;

        s_TerrainData.Pipeline->SetTexture(albedo, 1);
        s_TerrainData.Pipeline->SetTexture(emission, 2);
        s_TerrainData.Pipeline->SetTexture(metallic, 3);
        s_TerrainData.Pipeline->SetTexture(roughness, 4);
        s_TerrainData.Pipeline->SetTexture(normal, 5);
        s_TerrainData.Pipeline->SetTexture(ao, 6);
        // clang-format on

        s_TerrainData.Pipeline->Bind();
        RenderCommand::DrawIndexed(s_TerrainData.QuadIndexBuffer->GetCount());
    }
} // namespace Titan