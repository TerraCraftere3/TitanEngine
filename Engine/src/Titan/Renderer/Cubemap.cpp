#include "Cubemap.h"
#include "Renderer.h"
#include "Titan/Platform/OpenGL/OpenGLCubemap.h"

namespace Titan{

    Ref<Cubemap> Cubemap::Create(const std::string& path)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None:
                TI_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
                return nullptr;
            case RendererAPI::API::OpenGL:
                return CreateRef<OpenGLCubemap>(path);
        }

        TI_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

} // namespace Titan