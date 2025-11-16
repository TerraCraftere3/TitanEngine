#include "PBRRenderer.h"
#include "Buffer.h"
#include "RenderCommand.h"
#include "Shader.h"
#include "ShaderStorageBuffer.h"
#include "Titan/PCH.h"
#include "Titan/Scene/Assets.h"
#include "UniformBuffer.h"
#include "VertexArray.h"

namespace Titan
{

    struct PBRRendererData
    {
        Ref<Shader> Shader;
        Ref<UniformBuffer> SceneUniformBuffer;
        Ref<VertexArray> FullscreenQuadVAO;
        Ref<Cubemap> DefaultIrradiance;
    };

    static PBRRendererData s_PBRData;

    void PBRRenderer::Init()
    {
        s_PBRData.SceneUniformBuffer = UniformBuffer::Create(sizeof(PBRSceneData), 0);

        // Irradiance
        std::string defaultAmbientPath = "resources/cubemaps/default.hdr";
        Ref<Cubemap> defaultAmbient = Assets::Load<Cubemap>(defaultAmbientPath);
        s_PBRData.DefaultIrradiance = defaultAmbient->CreateIrradianceMap();

        // Shader
        s_PBRData.Shader = Shader::Create("assets/shader/RendererPBR.slang");

        // Vertex Array
        float quadVertices[] = {
            // positions   // texcoords
            -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f};

        uint32_t quadIndices[] = {0, 1, 2, 2, 3, 0};

        s_PBRData.FullscreenQuadVAO = VertexArray::Create();

        // Vertex Buffer
        Ref<VertexBuffer> vb = VertexBuffer::Create(quadVertices, sizeof(quadVertices));
        vb->SetLayout({{ShaderDataType::Float2, "a_Position"}, {ShaderDataType::Float2, "a_TexCoord"}});
        s_PBRData.FullscreenQuadVAO->AddVertexBuffer(vb);

        // Index Buffer
        Ref<IndexBuffer> ib = IndexBuffer::Create(quadIndices, sizeof(quadIndices) / sizeof(uint32_t));
        s_PBRData.FullscreenQuadVAO->SetIndexBuffer(ib);
    }

    void PBRRenderer::Shutdown()
    {
        s_PBRData = {};
    }

    void PBRRenderer::Render(Ref<Framebuffer> gbuffer, PBRSceneData data, Ref<Cubemap> irradiance)
    {
        s_PBRData.SceneUniformBuffer->SetData(&data, sizeof(PBRSceneData));

        s_PBRData.Shader->Bind();
        gbuffer->BindTexture(0, 1); // Position
        gbuffer->BindTexture(1, 2); // Normal
        gbuffer->BindTexture(2, 3); // Albedo
        gbuffer->BindTexture(3, 4); // Metallic, Roughness, AO, /
        gbuffer->BindTexture(4, 5); // Emission
        gbuffer->BindTexture(5, 6); // Entity ID
        gbuffer->BindDepthTexture(7);

        if (irradiance)
            irradiance->Bind(8);
        else
            s_PBRData.DefaultIrradiance->Bind(8);

        s_PBRData.Shader->SetInt("IrradianceMap", 8);

        s_PBRData.SceneUniformBuffer->Bind();

        RenderCommand::DrawIndexed(s_PBRData.FullscreenQuadVAO);
    }

} // namespace Titan
