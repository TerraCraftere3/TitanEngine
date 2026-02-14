#include "PBRRenderer.h"
#include "FullscreenRenderer.h"
#include "Titan/PCH.h"
#include "Titan/Platform/OpenGL/OpenGLShader.h"
#include "Titan/Renderer/RHI/Buffer.h"
#include "Titan/Renderer/RHI/PipelineState.h"
#include "Titan/Renderer/RHI/Shader.h"
#include "Titan/Renderer/RHI/UniformBuffer.h"
#include "Titan/Renderer/RenderCommand.h"
#include "Titan/Scene/Assets.h"

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
        s_PBRData.SceneUniformBuffer = UniformBuffer::Create(sizeof(PBRSceneData));

        // Irradiance
        std::string defaultAmbientPath = "resources/cubemaps/default.hdr";
        Ref<Cubemap> defaultAmbient = Assets::Load<Cubemap>(defaultAmbientPath);
        s_PBRData.DefaultIrradiance = defaultAmbient->CreateIrradianceMap();

        // Shader
        s_PBRData.Shader = Assets::Load<Shader>("resources/shader/RendererPBR.slang");
#ifdef TI_BUILD_DEBUG
        Assets::AttachHotreloader<Shader>("resources/shader/RendererPBR.slang");
#endif

        // Create pipeline
        s_PBRData.Pipeline = PipelineState::Create();
        s_PBRData.Pipeline->SetShader(s_PBRData.Shader);
        s_PBRData.Pipeline->SetUniformBuffer(s_PBRData.SceneUniformBuffer, 0);
    }

    void PBRRenderer::Shutdown()
    {
        s_PBRData.Pipeline.reset();
        s_PBRData = {};
    }

    void PBRRenderer::Render(Ref<Framebuffer> gbuffer, PBRSceneData data, Ref<Cubemap> irradiance)
    {
        TI_PROFILE_FUNCTION();
        s_PBRData.SceneUniformBuffer->SetData(&data, sizeof(PBRSceneData));

        s_PBRData.Pipeline->SetUniformBuffer(s_PBRData.SceneUniformBuffer, 0);

        // Bind textures and cubemap to the pipeline
        s_PBRData.Pipeline->SetTexture(gbuffer->GetColorAttachmentTexture(0), 0); // Position
        s_PBRData.Pipeline->SetTexture(gbuffer->GetColorAttachmentTexture(1), 1); // Normal
        s_PBRData.Pipeline->SetTexture(gbuffer->GetColorAttachmentTexture(2), 2); // Albedo
        s_PBRData.Pipeline->SetTexture(gbuffer->GetColorAttachmentTexture(3), 3); // Metallic, Roughness, AO
        s_PBRData.Pipeline->SetTexture(gbuffer->GetColorAttachmentTexture(4), 4); // Emission
        s_PBRData.Pipeline->SetTexture(gbuffer->GetColorAttachmentTexture(5), 5); // Entity ID
        s_PBRData.Pipeline->SetTexture(gbuffer->GetDepthAttachmentTexture(), 6);

        if (irradiance)
            s_PBRData.Pipeline->SetCubemap(irradiance, 7);
        else
            s_PBRData.Pipeline->SetCubemap(s_PBRData.DefaultIrradiance, 7);

        FullscreenRenderer::Render(s_PBRData.Pipeline);
    }

} // namespace Titan
