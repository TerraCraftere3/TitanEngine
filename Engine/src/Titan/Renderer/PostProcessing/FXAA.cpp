#include "FXAA.h"
#include "Titan/Renderer/FullscreenRenderer.h"

namespace Titan
{
    struct FXAAUniformData
    {
        glm::vec2 InvScreenSize;
    };

    void FXAAEffect::OnAttach()
    {
        m_Shader = Assets::Load<Shader>("resources/shader/PostFXAA.slang");
        m_UniformBuffer = UniformBuffer::Create(sizeof(FXAAUniformData), 0);
    }

    void FXAAEffect::OnDetach() {}

    void FXAAEffect::Execute(RenderGraph& graph, const RenderPass& pass, Ref<Framebuffer> framebuffer, Ref<Scene> scene,
                             PostFXComponent fxc)
    {
        if (!fxc.FXAASettings.isEnabled)
            return;

        FXAAUniformData data = {};
        data.InvScreenSize = glm::vec2(1.0f / (float)framebuffer->GetWidth(), 1.0f / (float)framebuffer->GetHeight());

        RenderCommand::BeginRenderPass(framebuffer, "Post Effect (FXAA)");
        framebuffer->GetColorAttachmentTexture(0)->Bind(0); // Color -> slot 0
        framebuffer->GetColorAttachmentTexture(1)->Bind(1); // EntityID -> slot 1
        m_Shader->Bind();
        m_Shader->SetInt("u_Input", 0);
        m_Shader->SetInt("u_Entity", 1);
        m_UniformBuffer->Bind();
        m_UniformBuffer->SetData(&data, sizeof(FXAAUniformData));
        FullscreenRenderer::Render();
        RenderCommand::EndRenderPass();
    }
} // namespace Titan