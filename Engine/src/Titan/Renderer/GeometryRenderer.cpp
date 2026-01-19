#include "GeometryRenderer.h"
#include "PipelineState.h"
#include "RenderCommand.h"
#include "Renderer2D.h"
#include "Shader.h"
#include "Titan/PCH.h"
#include "Titan/Platform/OpenGL/OpenGLShader.h"
#include "Titan/Scene/Assets.h"
#include "UniformBuffer.h"
#include "VertexArray.h"

namespace Titan
{
    glm::uvec2 HandleToVec2(uint64_t handle)
    {
        uint32_t low = static_cast<uint32_t>(handle & 0xFFFFFFFF);
        uint32_t high = static_cast<uint32_t>((handle >> 32) & 0xFFFFFFFF);
        return glm::uvec2(low, high);
    }

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

    struct Vertex
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec3 Tangent;
        glm::vec2 TexCoord;
    };

    // Per-instance data (uploaded once per mesh instance)
    struct alignas(16) InstanceData
    {
        glm::mat4 Transform;    // 64 bytes
        glm::mat4 NormalMatrix; // 64 bytes
        int EntityID;           // 4 bytes
        int MaterialIndex;      // 4 bytes
        float _pad[2];          // 8 bytes padding
    };

    struct alignas(16) GPUMaterial
    {
        glm::vec4 AlbedoColor;
        glm::uvec2 AlbedoTextureIndex;
        glm::uvec2 EmissionTextureIndex;
        glm::uvec2 MetallicTextureIndex;
        glm::uvec2 AOTextureIndex;
        glm::uvec2 RoughnessTextureIndex;
        glm::uvec2 NormalTextureIndex;
        glm::vec2 UVRepeat;

        GPUMaterial() = default;
        explicit GPUMaterial(const Material3D& mat)
        {
            AlbedoColor = mat.AlbedoColor;

            auto SetTextureHandle = [](glm::uvec2& target, Ref<Texture2D> tex, Ref<Texture2D> defaultTex)
            {
                Ref<Texture2D> useTex = tex ? tex : defaultTex;
                // Always remake resident to ensure handle is valid
                if (useTex->isValidBindlessHandle())
                    useTex->MakeHandleNonResident(); // Remove old handle
                useTex->MakeHandleResident();        // Create new handle
                target = HandleToVec2(useTex->GetBindlessHandle());
            };

            SetTextureHandle(AlbedoTextureIndex, mat.AlbedoTexture, s_Textures.DefaultAlbedo);
            SetTextureHandle(EmissionTextureIndex, mat.EmissionTexture, s_Textures.DefaultEmission);
            SetTextureHandle(MetallicTextureIndex, mat.MetallicTexture, s_Textures.DefaultMetallic);
            SetTextureHandle(RoughnessTextureIndex, mat.RoughnessTexture, s_Textures.DefaultRoughness);
            SetTextureHandle(NormalTextureIndex, mat.NormalTexture, s_Textures.DefaultNormal);
            SetTextureHandle(AOTextureIndex, mat.AOTexture, s_Textures.DefaultAO);

            UVRepeat = mat.UVRepeat;
        }
    };

    // Submesh represents a range of indices with one material
    struct Submesh
    {
        uint32_t BaseIndex;
        uint32_t IndexCount;
        uint32_t MaterialIndex;
    };

    // Cached mesh GPU data
    struct MeshGPUData
    {
        Ref<VertexArray> VAO;
        Ref<VertexBuffer> VBO;
        Ref<IndexBuffer> IBO;
        uint32_t TotalIndexCount;
        std::vector<Submesh> Submeshes; // Each submesh has its own material
    };

    struct GeometryRendererData
    {
        static const uint32_t MaxInstances = 10000;
        static const uint32_t MaxMaterials = 1000;

        std::vector<InstanceData> InstanceBuffer;
        Ref<ShaderStorageBuffer> InstanceSSBO;

        Ref<Mesh> CurrentMesh;
        MeshGPUData* CurrentMeshGPU = nullptr;
        uint32_t CurrentInstanceCount = 0;

        struct CameraData
        {
            glm::mat4 ViewProjection;
        };
        CameraData CamBuffer;

        Ref<Shader> Shader;
        Ref<PipelineState> Pipeline;
        Ref<UniformBuffer> CameraUniformBuffer;
        Ref<ShaderStorageBuffer> MaterialStorageBuffer;

        std::vector<GPUMaterial> GPUMaterials;
        std::unordered_map<size_t, uint32_t> MaterialIndexMap;

        std::unordered_map<Mesh*, MeshGPUData> MeshCache;

        GeometryRenderer::Statistics Stats;
    };

    static GeometryRendererData s_3DData;
    static bool s_IsRendering = false;

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

    static uint32_t GetOrAddMaterial(const Material3D& mat)
    {
        size_t hash = HashMaterial(mat);
        auto it = s_3DData.MaterialIndexMap.find(hash);
        if (it != s_3DData.MaterialIndexMap.end())
            return it->second;

        uint32_t newIndex = static_cast<uint32_t>(s_3DData.GPUMaterials.size());
        if (newIndex >= s_3DData.MaxMaterials)
        {
            TI_CORE_ERROR("Material limit reached!");
            return 0;
        }

        s_3DData.GPUMaterials.push_back(GPUMaterial(mat));
        s_3DData.MaterialIndexMap[hash] = newIndex;
        return newIndex;
    }

    MeshGPUData* GetOrCreateMeshGPUData(Mesh* mesh)
    {
        auto it = s_3DData.MeshCache.find(mesh);
        if (it != s_3DData.MeshCache.end())
            return &it->second;

        MeshGPUData gpuData;

        const auto& positions = mesh->GetPositions();
        const auto& normals = mesh->GetNormals();
        const auto& tangents = mesh->GetTangents();
        const auto& texCoords = mesh->GetTexCoords();
        const auto& indices = mesh->GetIndices();
        const auto& matIndices = mesh->GetMaterialIndices(); // Per-vertex material index
        const auto& materials = mesh->GetMaterials();

        std::vector<Vertex> vertices;
        vertices.reserve(positions.size());

        for (size_t i = 0; i < positions.size(); ++i)
        {
            Vertex v;
            v.Position = positions[i];
            v.Normal = normals[i];
            v.Tangent = tangents[i];
            v.TexCoord = texCoords[i];
            vertices.push_back(v);
        }

        gpuData.VAO = VertexArray::Create();
        gpuData.VBO = VertexBuffer::Create(vertices.data(), vertices.size() * sizeof(Vertex));

        gpuData.VBO->SetLayout({{ShaderDataType::Float3, "a_Position"},
                                {ShaderDataType::Float3, "a_Normal"},
                                {ShaderDataType::Float3, "a_Tangent"},
                                {ShaderDataType::Float2, "a_TexCoord"}});

        gpuData.VAO->AddVertexBuffer(gpuData.VBO);

        if (!indices.empty() && !matIndices.empty())
        {
            std::unordered_map<uint8_t, std::vector<uint32_t>> materialGroups;

            for (size_t i = 0; i < indices.size(); i += 3)
            {
                uint8_t matIdx = matIndices[indices[i]];
                materialGroups[matIdx].push_back(indices[i]);
                materialGroups[matIdx].push_back(indices[i + 1]);
                materialGroups[matIdx].push_back(indices[i + 2]);
            }

            std::vector<uint32_t> finalIndices;
            for (const auto& [localMatIdx, groupIndices] : materialGroups)
            {
                Submesh submesh;
                submesh.BaseIndex = finalIndices.size();
                submesh.IndexCount = groupIndices.size();
                submesh.MaterialIndex = GetOrAddMaterial(*materials[localMatIdx]);

                finalIndices.insert(finalIndices.end(), groupIndices.begin(), groupIndices.end());
                gpuData.Submeshes.push_back(submesh);
            }

            gpuData.IBO = IndexBuffer::Create(finalIndices.data(), finalIndices.size());
            gpuData.VAO->SetIndexBuffer(gpuData.IBO);
            gpuData.TotalIndexCount = finalIndices.size();
        }
        else
        {
            std::vector<uint32_t> generatedIndices;
            generatedIndices.reserve(positions.size());
            for (uint32_t i = 0; i < positions.size(); ++i)
                generatedIndices.push_back(i);

            gpuData.IBO = IndexBuffer::Create(generatedIndices.data(), generatedIndices.size());
            gpuData.VAO->SetIndexBuffer(gpuData.IBO);
            gpuData.TotalIndexCount = generatedIndices.size();

            Submesh submesh;
            submesh.BaseIndex = 0;
            submesh.IndexCount = generatedIndices.size();
            submesh.MaterialIndex = materials.empty() ? 0 : GetOrAddMaterial(*materials[0]);
            gpuData.Submeshes.push_back(submesh);
        }

        s_3DData.MeshCache[mesh] = std::move(gpuData);
        return &s_3DData.MeshCache[mesh];
    }

    void GeometryRenderer::Init()
    {
        s_3DData.InstanceBuffer.reserve(s_3DData.MaxInstances);

        s_3DData.CameraUniformBuffer = UniformBuffer::Create(sizeof(GeometryRendererData::CameraData), 0);
        s_3DData.MaterialStorageBuffer = ShaderStorageBuffer::Create(sizeof(GPUMaterial) * s_3DData.MaxMaterials, 1);
        s_3DData.InstanceSSBO = ShaderStorageBuffer::Create(sizeof(InstanceData) * s_3DData.MaxInstances, 2);

        s_3DData.Shader = Shader::Create("resources/shader/RendererGeometry.slang");

        // Create pipeline state
        s_3DData.Pipeline = PipelineState::Create();
        s_3DData.Pipeline->SetShader(s_3DData.Shader);
        s_3DData.Pipeline->BindUniformBuffer(s_3DData.CameraUniformBuffer, 0);
        s_3DData.Pipeline->BindShaderStorageBuffer(s_3DData.MaterialStorageBuffer, 1);
        s_3DData.Pipeline->BindShaderStorageBuffer(s_3DData.InstanceSSBO, 2);

        s_3DData.GPUMaterials.reserve(s_3DData.MaxMaterials);

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

    void GeometryRenderer::Shutdown()
    {
        s_3DData.MeshCache.clear();
        s_3DData.InstanceBuffer.clear();
        s_3DData.GPUMaterials.clear();
        s_3DData.MaterialIndexMap.clear();
        s_3DData.Pipeline.reset();
        s_Textures = {};
    }

    void GeometryRenderer::ClearCache()
    {
        s_3DData.MeshCache.clear();
        s_3DData.GPUMaterials.clear();
        s_3DData.MaterialIndexMap.clear();
    }

    void GeometryRenderer::ClearTextureCache()
    {
        s_3DData.GPUMaterials.clear();
    }

    void GeometryRenderer::ClearMeshCache()
    {
        s_3DData.MeshCache.clear();
    }

    void GeometryRenderer::BeginScene(const glm::mat4& viewProjectionMatrix)
    {
        TI_CORE_ASSERT(!s_IsRendering, "Forgot to call GeometryRenderer::EndScene()?");

        s_3DData.CamBuffer.ViewProjection = viewProjectionMatrix;
        s_3DData.CameraUniformBuffer->SetData(&s_3DData.CamBuffer, sizeof(GeometryRendererData::CameraData));

        s_3DData.CurrentMesh = nullptr;
        s_3DData.CurrentMeshGPU = nullptr;
        s_3DData.CurrentInstanceCount = 0;
        s_3DData.InstanceBuffer.clear();

        s_IsRendering = true;
    }

    void GeometryRenderer::EndScene()
    {
        TI_CORE_ASSERT(s_IsRendering, "Called EndScene() without BeginScene()");

        Flush(); // Flush remaining instances
        s_IsRendering = false;

        // Upload all materials once per frame
        if (!s_3DData.GPUMaterials.empty())
        {
            s_3DData.MaterialStorageBuffer->SetData(s_3DData.GPUMaterials.data(),
                                                    s_3DData.GPUMaterials.size() * sizeof(GPUMaterial));
        }
        s_3DData.GPUMaterials.clear();
        s_3DData.MaterialIndexMap.clear();
    }

    void GeometryRenderer::Flush()
    {
        if (s_3DData.CurrentInstanceCount == 0 || !s_3DData.CurrentMeshGPU)
            return;

        // Upload instance data
        s_3DData.InstanceSSBO->SetData(s_3DData.InstanceBuffer.data(),
                                       s_3DData.CurrentInstanceCount * sizeof(InstanceData));

        // Set the VAO in the pipeline
        s_3DData.Pipeline->SetVertexArray(s_3DData.CurrentMeshGPU->VAO);
        s_3DData.Pipeline->Bind();

        // Draw each submesh with instancing
        for (const auto& submesh : s_3DData.CurrentMeshGPU->Submeshes)
        {
            for (uint32_t i = 0; i < s_3DData.CurrentInstanceCount; ++i)
            {
                s_3DData.InstanceBuffer[i].MaterialIndex = submesh.MaterialIndex;
            }

            s_3DData.InstanceSSBO->SetData(s_3DData.InstanceBuffer.data(),
                                           s_3DData.CurrentInstanceCount * sizeof(InstanceData));

            RenderCommand::DrawIndexedInstancedBaseIndex(submesh.IndexCount, s_3DData.CurrentInstanceCount,
                                                         submesh.BaseIndex);

            s_3DData.Stats.DrawCalls++;
        }

        s_3DData.Stats.VertexCount += s_3DData.CurrentMeshGPU->TotalIndexCount * s_3DData.CurrentInstanceCount;

        s_3DData.CurrentInstanceCount = 0;
        s_3DData.InstanceBuffer.clear();
    }

    void GeometryRenderer::DrawMesh(const Ref<Mesh>& mesh, const glm::mat4& transform, int entityID)
    {
        TI_CORE_ASSERT(s_IsRendering, "Must call BeginScene() before DrawMesh()");
        if (!mesh)
            return;

        MeshGPUData* meshGPU = GetOrCreateMeshGPUData(mesh.get());

        if (s_3DData.CurrentMesh.get() != mesh.get() || s_3DData.CurrentInstanceCount >= s_3DData.MaxInstances)
        {
            Flush();
            s_3DData.CurrentMesh = mesh;
            s_3DData.CurrentMeshGPU = meshGPU;
        }

        InstanceData instance;
        instance.Transform = transform;
        instance.NormalMatrix = glm::transpose(glm::inverse(transform));
        instance.EntityID = entityID;
        instance.MaterialIndex = 0;

        s_3DData.InstanceBuffer.push_back(instance);
        s_3DData.CurrentInstanceCount++;
        s_3DData.Stats.MeshCount++;
    }

    GeometryRenderer::Statistics GeometryRenderer::GetStats()
    {
        return s_3DData.Stats;
    }

    void GeometryRenderer::ResetStats()
    {
        memset(&s_3DData.Stats, 0, sizeof(Statistics));
    }

} // namespace Titan