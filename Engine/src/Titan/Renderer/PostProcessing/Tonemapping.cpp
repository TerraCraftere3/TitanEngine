#include "Tonemapping.h"
#include "Titan/Renderer/FullscreenRenderer.h"

namespace Titan
{
    struct TonemappingUniformData
    {
        int Operator;     // Which tonemapping operator to use
        float Exposure;   // Exposure adjustment
        float Gamma;      // Gamma correction value
        float WhitePoint; // White point for certain operators
    };

    void TonemappingEffect::OnAttach()
    {
        m_Shader = Assets::Load<Shader>("resources/shader/PostTonemapping.slang");
        m_UniformBuffer = UniformBuffer::Create(sizeof(TonemappingUniformData), 0);
    }

    void TonemappingEffect::OnDetach() {}

    void TonemappingEffect::Execute(RenderGraph& graph, const RenderPass& pass, Ref<Framebuffer> framebuffer,
                                    Ref<Scene> scene, PostFXComponent fxc)
    {
        if (!fxc.TonemappingSettings.isEnabled)
            return;

        TonemappingUniformData data = {};
        data.Operator = static_cast<int>(fxc.TonemappingSettings.Operator);
        data.Exposure = fxc.TonemappingSettings.Exposure;
        data.Gamma = fxc.TonemappingSettings.Gamma;
        data.WhitePoint = fxc.TonemappingSettings.WhitePoint;

        RenderCommand::BeginRenderPass(framebuffer);
        framebuffer->GetColorAttachmentTexture(0)->Bind(0); // HDR Color -> slot 0
        framebuffer->GetColorAttachmentTexture(1)->Bind(1); // EntityID -> slot 1
        m_Shader->Bind();
        m_Shader->SetInt("u_HDRInput", 0);
        m_Shader->SetInt("u_Entity", 1);
        m_UniformBuffer->Bind();
        m_UniformBuffer->SetData(&data, sizeof(TonemappingUniformData));
        FullscreenRenderer::Render();
        RenderCommand::EndRenderPass();
    }
} // namespace Titan
