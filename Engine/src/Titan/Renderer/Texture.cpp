#include "Texture.h"
#include "Titan/PCH.h"
#include "Titan/Platform/OpenGL/OpenGLTexture.h"
#include "Titan/Renderer/Renderer.h"

namespace Titan
{
    Ref<Texture2D> Texture2D::Create(uint32_t width, uint32_t height)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None:
                TI_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
                return nullptr;
            case RendererAPI::API::OpenGL:
                return CreateRef<OpenGLTexture2D>(width, height);
        }

        TI_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    Ref<Texture2D> Texture2D::Create(const std::string& path, TextureSettings settings)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None:
                TI_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
                return nullptr;
            case RendererAPI::API::OpenGL:
                return CreateRef<OpenGLTexture2D>(path, settings);
        }

        TI_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

} // namespace Titan