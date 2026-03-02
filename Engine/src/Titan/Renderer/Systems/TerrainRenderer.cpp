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
        uint32_t LODIndex;
        float _pad1[2];
        glm::vec4 AlbedoColor;
        glm::vec2 UVRepeat;
        glm::vec2 UV0;
        glm::vec2 UV1;
    };

    struct TerrainLOD
    {
        float DistanceThreshold;
        uint32_t VertexDensity;
        Ref<VertexArray> VertexArray;
        Ref<VertexBuffer> VertexBuffer;
        Ref<IndexBuffer> IndexBuffer;
    };

    struct TerrainRendererData
    {
        Ref<Shader> Shader;
        Ref<PipelineState> Pipeline;
        Ref<UniformBuffer> UniformBuffer;
        UniformBufferObject UBO;
        std::vector<TerrainLOD> LODs;
        glm::vec3 CameraPosition;
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

        struct QuadCreationDescription
        {
            float DistanceThreshold;
            uint32_t VertexDensity;
        };

        // clang-format off
        std::vector<QuadCreationDescription> quadDescriptions = {
            { 5.0f, 256 },
            { 10.0f, 128 },
            { 50.0f, 64 },
            { 200.0f, 32 },
            { 300.0f, 16},
            { std::numeric_limits<float>::max(), 8 }
        };
        // clang-format on

        for (const auto& desc : quadDescriptions)
        {
            TerrainLOD lod;
            lod.DistanceThreshold = desc.DistanceThreshold;
            lod.VertexDensity = desc.VertexDensity;
            lod.VertexArray = VertexArray::Create();

            auto quadMesh = QuadMeshGenerator::GenerateQuad(desc.VertexDensity, desc.VertexDensity, 1.0f, 1.0f);
            lod.VertexBuffer = VertexBuffer::Create(quadMesh.Vertices.size() * sizeof(QuadMeshVertex));
            lod.VertexBuffer->SetLayout({{ShaderDataType::Float3, "a_Position"},
                                         {ShaderDataType::Float3, "a_Normal"},
                                         {ShaderDataType::Float3, "a_Tangent"},
                                         {ShaderDataType::Float2, "a_TexCoord"}});

            lod.VertexBuffer->SetData(quadMesh.Vertices.data(), quadMesh.Vertices.size() * sizeof(QuadMeshVertex));
            lod.VertexArray->AddVertexBuffer(lod.VertexBuffer);

            lod.IndexBuffer = IndexBuffer::Create(quadMesh.Indices.data(), quadMesh.Indices.size());
            lod.VertexArray->SetIndexBuffer(lod.IndexBuffer);
            s_TerrainData.LODs.push_back(lod);
        }

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
        s_TerrainData = {};
        s_Textures = {};
    }

    void TerrainRenderer::BeginScene(const glm::mat4& viewTransform, const glm::vec3& cameraPosition,
                                     PolygonMode polygonMode)
    {
        s_TerrainData.Pipeline->SetPolygonMode(polygonMode);
        s_TerrainData.UBO.ViewProjection = viewTransform;
        s_TerrainData.CameraPosition = cameraPosition;
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

        // clang-format off
        auto albedo    = material && material->AlbedoTexture    ? material->AlbedoTexture    : s_Textures.DefaultAlbedo;
        auto emission  = material && material->EmissionTexture  ? material->EmissionTexture  : s_Textures.DefaultEmission;
        auto metallic  = material && material->MetallicTexture  ? material->MetallicTexture  : s_Textures.DefaultMetallic;
        auto roughness = material && material->RoughnessTexture ? material->RoughnessTexture : s_Textures.DefaultRoughness;
        auto normal    = material && material->NormalTexture    ? material->NormalTexture    : s_Textures.DefaultNormal;
        auto ao        = material && material->AOTexture        ? material->AOTexture        : s_Textures.DefaultAO;

        s_TerrainData.Pipeline->SetTexture(albedo, 1);
        s_TerrainData.Pipeline->SetTexture(emission, 2);
        s_TerrainData.Pipeline->SetTexture(normal, 3);
        s_TerrainData.Pipeline->SetTexture(metallic, 4);
        s_TerrainData.Pipeline->SetTexture(roughness, 5);
        s_TerrainData.Pipeline->SetTexture(ao, 6);
        // clang-format on

        // Base Units
        glm::vec3 translation;
        translation.x = transform[3][0];
        translation.y = transform[3][1];
        translation.z = transform[3][2];
        glm::vec3 scale;
        scale.x = glm::length(glm::vec3(transform[0]));
        scale.y = glm::length(glm::vec3(transform[1]));
        scale.z = glm::length(glm::vec3(transform[2]));

        int subdivisionSize = 25;
        int subdivX = int(scale.x / subdivisionSize);
        int subdivZ = int(scale.z / subdivisionSize);

        for (int z = 0; z < subdivZ; ++z)
        {
            for (int x = 0; x < subdivX; ++x)
            {
                glm::mat4 subTransform = transform;
                // Position the sub-quad in the correct location
                subTransform[3][0] = translation.x + (x - subdivX / 2) * (float)subdivisionSize;
                subTransform[3][2] = translation.z + (z - subdivZ / 2) * (float)subdivisionSize;

                // Scale the sub-quad to the correct size
                subTransform[0] = glm::normalize(subTransform[0]) * (float)subdivisionSize;
                subTransform[2] = glm::normalize(subTransform[2]) * (float)subdivisionSize;

                // Set UV coordinates for this strip
                // UV0 = normalized position of this strip in the subdivision grid
                s_TerrainData.UBO.UV0 = glm::vec2(x / (float)subdivX, z / (float)subdivZ);
                // UV1 = scale of one subdivision in UV space
                s_TerrainData.UBO.UV1 = glm::vec2(1.0f / subdivX, 1.0f / subdivZ);

                s_TerrainData.UBO.Transform = subTransform;

                TerrainLOD* selectedLOD = nullptr;
                uint32_t selectedLODIndex = 0;
                for (size_t lodIndex = 0; lodIndex < s_TerrainData.LODs.size(); ++lodIndex)
                {
                    auto& lod = s_TerrainData.LODs[lodIndex];
                    if (glm::length(glm::vec3(subTransform[3]) - s_TerrainData.CameraPosition) < lod.DistanceThreshold)
                    {
                        selectedLOD = &lod;
                        selectedLODIndex = static_cast<uint32_t>(lodIndex);
                        break;
                    }
                }

                if (!selectedLOD)
                    return;
                s_TerrainData.UBO.LODIndex = selectedLODIndex;
                s_TerrainData.Pipeline->SetVertexArray(selectedLOD->VertexArray);
                s_TerrainData.UniformBuffer->SetData(&s_TerrainData.UBO, sizeof(UniformBufferObject));
                s_TerrainData.Pipeline->Bind();
                RenderCommand::DrawIndexed(selectedLOD->IndexBuffer->GetCount());
                s_TerrainData.Pipeline->ClearBindings();
            }
        };
    }
} // namespace Titan