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
        m_UniformBuffer = UniformBuffer::Create(sizeof(TonemappingUniformData));
    }

    void TonemappingEffect::OnDetach() {}

    void TonemappingEffect::Render(const PostFXInput& data)
    {
        if (!data.fxc.TonemappingSettings.isEnabled)
            return;

        TonemappingUniformData uniformData = {};
        uniformData.Operator = static_cast<int>(data.fxc.TonemappingSettings.Operator);
        uniformData.Exposure = data.fxc.TonemappingSettings.Exposure;
        uniformData.Gamma = data.fxc.TonemappingSettings.Gamma;
        uniformData.WhitePoint = data.fxc.TonemappingSettings.WhitePoint;

        RenderCommand::BeginRenderPass(data.output, "Post Effect (Tonemapping)");

        data.input->GetColorAttachmentTexture(0)->Bind(0); // HDR Color -> slot 0
        data.input->GetColorAttachmentTexture(1)->Bind(1); // EntityID -> slot 1

        m_Shader->Bind();
        m_Shader->SetInt("u_HDRInput", 0);
        m_Shader->SetInt("u_Entity", 1);

        m_UniformBuffer->Bind(0);
        m_UniformBuffer->SetData(&uniformData, sizeof(TonemappingUniformData));

        RenderCommand::Clear();
        FullscreenRenderer::Render();
        RenderCommand::EndRenderPass();
    }
} // namespace Titan
