#include "Titan/Renderer/VertexArray.h"
#include "Titan/PCH.h"

#include "Titan/Platform/OpenGL/OpenGLVertexArray.h"
#include "Titan/Renderer/Renderer.h"

namespace Titan
{

    Ref<VertexArray> VertexArray::Create()
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None:
                TI_CORE_ASSERT(false, "Graphics API None is currently not supported!");
                return nullptr;
            case RendererAPI::API::OpenGL:
                return CreateRef<OpenGLVertexArray>();
        }

        TI_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

} // namespace Titan