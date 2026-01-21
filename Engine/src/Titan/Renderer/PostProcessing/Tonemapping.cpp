#include "Tonemapping.h"
#include "Titan/Renderer/Blit.h"
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
        m_UniformBuffer = UniformBuffer::Create(sizeof(TonemappingUniformData));
        m_Pipeline = PipelineState::Create();
        m_Pipeline->SetShader(m_Shader);
    }

    void TonemappingEffect::OnDetach() {}

    void TonemappingEffect::Render(const PostFXInput& data)
    {
        if (!data.fxc.TonemappingSettings.isEnabled)
        {
            Blit::Execute(data.input, data.output, BlitMode::ColorAndEntity);
            return;
        }

        TonemappingUniformData uniformData = {};
        uniformData.Operator = static_cast<int>(data.fxc.TonemappingSettings.Operator);
        uniformData.Exposure = data.fxc.TonemappingSettings.Exposure;
        uniformData.Gamma = data.fxc.TonemappingSettings.Gamma;
        uniformData.WhitePoint = data.fxc.TonemappingSettings.WhitePoint;

        RenderCommand::BeginRenderPass(data.output, "Post Effect (Tonemapping)");

        m_UniformBuffer->SetData(&uniformData, sizeof(TonemappingUniformData));

        m_Pipeline->SetUniformBuffer(m_UniformBuffer, 0);
        m_Pipeline->SetTexture(data.input->GetColorAttachmentTexture(0), 0); // HDR Color -> slot 0
        m_Pipeline->SetTexture(data.input->GetColorAttachmentTexture(1), 1); // EntityID -> slot 1

        RenderCommand::Clear();
        FullscreenRenderer::Render(m_Pipeline);
        RenderCommand::EndRenderPass();
    }
} // namespace Titan
