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
        m_UniformBuffer = UniformBuffer::Create(sizeof(FXAAUniformData));
        m_Pipeline = PipelineState::Create();
        m_Pipeline->SetShader(m_Shader);
    }

    void FXAAEffect::OnDetach() {}

    void FXAAEffect::Render(const PostFXInput& data)
    {
        if (!data.fxc.FXAASettings.isEnabled)
            return;

        RenderCommand::BeginRenderPass(data.output, "Post Effect (FXAA)");

        FXAAUniformData uniformData = {};
        uniformData.InvScreenSize =
            glm::vec2(1.0f / (float)data.output->GetWidth(), 1.0f / (float)data.output->GetHeight());

        m_UniformBuffer->SetData(&uniformData, sizeof(FXAAUniformData));

        m_Pipeline->BindUniformBuffer(m_UniformBuffer, 0);
        m_Pipeline->BindTexture(data.input->GetColorAttachmentTexture(0), 0); // Color -> slot 0
        m_Pipeline->BindTexture(data.input->GetColorAttachmentTexture(1), 1); // EntityID -> slot 1

        m_Shader->Bind();
        m_Shader->SetInt("u_Input", 0);
        m_Shader->SetInt("u_Entity", 1);

        RenderCommand::Clear();
        FullscreenRenderer::Render(m_Pipeline);

        RenderCommand::EndRenderPass();
    }
} // namespace Titan