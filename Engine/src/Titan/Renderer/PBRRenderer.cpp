#include "PBRRenderer.h"
#include "Buffer.h"
#include "FullscreenRenderer.h"
#include "PipelineState.h"
#include "RenderCommand.h"
#include "Shader.h"
#include "Titan/PCH.h"
#include "Titan/Platform/OpenGL/OpenGLShader.h"
#include "Titan/Scene/Assets.h"
#include "UniformBuffer.h"

namespace Titan
{

    struct PBRRendererData
    {
        Ref<Shader> Shader;
        Ref<PipelineState> Pipeline;
        Ref<UniformBuffer> SceneUniformBuffer;
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
        s_PBRData.Shader = Shader::Create("resources/shader/RendererPBR.slang");

        // Create pipeline
        s_PBRData.Pipeline = PipelineState::Create();
        s_PBRData.Pipeline->SetShader(s_PBRData.Shader);
        s_PBRData.Pipeline->BindUniformBuffer(s_PBRData.SceneUniformBuffer, 0);
    }

    void PBRRenderer::Shutdown()
    {
        s_PBRData.Pipeline.reset();
        s_PBRData = {};
    }

    void PBRRenderer::Render(Ref<Framebuffer> gbuffer, PBRSceneData data, Ref<Cubemap> irradiance)
    {
        s_PBRData.SceneUniformBuffer->SetData(&data, sizeof(PBRSceneData));

        // Bind textures and cubemap to the pipeline
        s_PBRData.Pipeline->BindTexture(gbuffer->GetColorAttachmentTexture(0), 1); // Position
        s_PBRData.Pipeline->BindTexture(gbuffer->GetColorAttachmentTexture(1), 2); // Normal
        s_PBRData.Pipeline->BindTexture(gbuffer->GetColorAttachmentTexture(2), 3); // Albedo
        s_PBRData.Pipeline->BindTexture(gbuffer->GetColorAttachmentTexture(3), 4); // Metallic, Roughness, AO
        s_PBRData.Pipeline->BindTexture(gbuffer->GetColorAttachmentTexture(4), 5); // Emission
        s_PBRData.Pipeline->BindTexture(gbuffer->GetColorAttachmentTexture(5), 6); // Entity ID
        s_PBRData.Pipeline->BindTexture(gbuffer->GetDepthAttachmentTexture(), 7);

        if (irradiance)
            s_PBRData.Pipeline->BindCubemap(irradiance, 8);
        else
            s_PBRData.Pipeline->BindCubemap(s_PBRData.DefaultIrradiance, 8);

        // Set uniforms on the shader directly
        {
            std::dynamic_pointer_cast<OpenGLShader>(s_PBRData.Shader)->Bind();
            s_PBRData.Shader->SetInt("IrradianceMap", 8);
        }

        s_PBRData.Pipeline->Bind();

        FullscreenRenderer::Render();
    }

} // namespace Titan
