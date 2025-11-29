#include "PBRRenderer.h"
#include "Buffer.h"
#include "FullscreenRenderer.h"
#include "RenderCommand.h"
#include "Shader.h"
#include "ShaderStorageBuffer.h"
#include "Titan/PCH.h"
#include "Titan/Scene/Assets.h"
#include "UniformBuffer.h"

namespace Titan
{

    struct PBRRendererData
    {
        Ref<Shader> Shader;
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
    }

    void PBRRenderer::Shutdown()
    {
        s_PBRData = {};
    }

    void PBRRenderer::Render(Ref<Framebuffer> gbuffer, PBRSceneData data, Ref<Cubemap> irradiance)
    {
        s_PBRData.SceneUniformBuffer->SetData(&data, sizeof(PBRSceneData));

        s_PBRData.Shader->Bind();
        gbuffer->GetColorAttachmentTexture(0)->Bind(1); // Position
        gbuffer->GetColorAttachmentTexture(1)->Bind(2); // Normal
        gbuffer->GetColorAttachmentTexture(2)->Bind(3); // Albedo
        gbuffer->GetColorAttachmentTexture(3)->Bind(4); // Metallic, Roughness, AO
        gbuffer->GetColorAttachmentTexture(4)->Bind(5); // Emission
        gbuffer->GetColorAttachmentTexture(5)->Bind(6); // Entity ID
        gbuffer->GetDepthAttachmentTexture()->Bind(7);

        if (irradiance)
            irradiance->Bind(8);
        else
            s_PBRData.DefaultIrradiance->Bind(8);

        s_PBRData.Shader->SetInt("IrradianceMap", 8);

        s_PBRData.SceneUniformBuffer->Bind();

        FullscreenRenderer::Render();
    }

} // namespace Titan
