#include "PipelineState.h"
#include "Titan/Platform/OpenGL/OpenGLPipelineState.h"
#include "RendererAPI.h"

namespace Titan
{
    Ref<PipelineState> PipelineState::Create()
    {
        switch (RendererAPI::GetAPI())
        {
            case RendererAPI::API::OpenGL:
                return CreateRef<OpenGLPipelineState>();
            case RendererAPI::API::None:
                TI_CORE_ASSERT(false, "RendererAPI::API::None is currently not supported!");
                return nullptr;
        }

        TI_CORE_ASSERT(false, "Unknown RendererAPI::API!");
        return nullptr;
    }
} // namespace Titan
