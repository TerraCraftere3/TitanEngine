#include "Blit.h"
#include "FullscreenRenderer.h"
#include "RenderCommand.h"
#include "Titan/Platform/OpenGL/OpenGLShader.h"

namespace Titan
{
    Blit::BlitData Blit::s_Data;

    void Blit::Init()
    {
        s_Data.Shader = Shader::Create("resources/shader/Blit.slang");
        s_Data.Pipeline = PipelineState::Create();
        s_Data.Pipeline->SetShader(s_Data.Shader);
    }

    void Blit::Shutdown()
    {
        s_Data.Pipeline.reset();
        s_Data.Shader.reset();
        s_Data = {};
    }

    void Blit::Execute(const Ref<Framebuffer>& source, const Ref<Framebuffer>& destination, BlitMode mode)
    {
        TI_CORE_ASSERT(source, "Source framebuffer is null");
        TI_CORE_ASSERT(destination, "Destination framebuffer is null");

        RenderCommand::BeginRenderPass(destination, "Blit");

        s_Data.Pipeline->BindTexture(source->GetColorAttachmentTexture(0), 0);
        if (mode == BlitMode::ColorAndEntity)
            s_Data.Pipeline->BindTexture(source->GetColorAttachmentTexture(1), 1);

        {
            std::dynamic_pointer_cast<OpenGLShader>(s_Data.Shader)->Bind();
            s_Data.Shader->SetInt("u_ColorInput", 0);
            s_Data.Shader->SetInt("u_EntityInput", 1);
            s_Data.Shader->SetInt("u_BlitMode", static_cast<int>(mode));
        }

        RenderCommand::Clear();
        FullscreenRenderer::Render(s_Data.Pipeline);
        RenderCommand::EndRenderPass();
    }

} // namespace Titan
