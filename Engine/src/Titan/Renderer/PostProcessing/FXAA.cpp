#include "FXAA.h"
#include "Titan/Renderer/Blit.h"
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
        {
            Blit::Execute(data.input, data.output, BlitMode::ColorAndEntity);
            return;
        }

        RenderCommand::BeginRenderPass(data.output, "Post Effect (FXAA)");

        FXAAUniformData uniformData = {};
        uniformData.InvScreenSize =
            glm::vec2(1.0f / (float)data.output->GetWidth(), 1.0f / (float)data.output->GetHeight());

        m_UniformBuffer->SetData(&uniformData, sizeof(FXAAUniformData));

        m_Pipeline->SetUniformBuffer(m_UniformBuffer, 0);
        m_Pipeline->SetTexture(data.input->GetColorAttachmentTexture(0), 0); // Color -> slot 0
        m_Pipeline->SetTexture(data.input->GetColorAttachmentTexture(1), 1); // EntityID -> slot 1

        RenderCommand::Clear({1.0f, 0.0f, 1.0f, 1.0f});
        FullscreenRenderer::Render(m_Pipeline);

        RenderCommand::EndRenderPass();
    }
} // namespace Titan