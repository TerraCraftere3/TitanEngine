#include "Blit.h"
#include "Titan/Renderer/RenderCommand.h"
#include "Titan/Renderer/Systems/FullscreenRenderer.h"

namespace Titan
{
    struct BlitUniformData
    {
        int BlitMode;
        int _pad0;
        int _pad1;
        int _pad2;
    };

    Blit::BlitData Blit::s_Data;

    void Blit::Init()
    {
        s_Data.Shader = Shader::Create("resources/shader/Blit.slang");
        s_Data.UniformBuffer = UniformBuffer::Create(sizeof(BlitUniformData));
        s_Data.Pipeline = PipelineState::Create();
        s_Data.Pipeline->SetShader(s_Data.Shader);
    }

    void Blit::Shutdown()
    {
        s_Data.Pipeline.reset();
        s_Data.Shader.reset();
        s_Data.UniformBuffer.reset();
        s_Data = {};
    }

    void Blit::Execute(const Ref<Framebuffer>& source, const Ref<Framebuffer>& destination, BlitMode mode)
    {
        TI_CORE_ASSERT(source, "Source framebuffer is null");
        TI_CORE_ASSERT(destination, "Destination framebuffer is null");

        RenderCommand::BeginRenderPass(destination, "Blit");

        BlitUniformData uniformData = {};
        uniformData.BlitMode = static_cast<int>(mode);

        s_Data.UniformBuffer->SetData(&uniformData, sizeof(BlitUniformData));

        s_Data.Pipeline->SetUniformBuffer(s_Data.UniformBuffer, 0);
        s_Data.Pipeline->SetTexture(source->GetColorAttachmentTexture(0), 0);
        if (mode == BlitMode::ColorAndEntity)
            s_Data.Pipeline->SetTexture(source->GetColorAttachmentTexture(1), 1);

        RenderCommand::Clear({0.0f, 0.0f, 0.0f, 1.0f});
        FullscreenRenderer::Render(s_Data.Pipeline);
        RenderCommand::EndRenderPass();
    }

} // namespace Titan
