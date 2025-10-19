#include "Titan/Renderer/Shader.h"
#include "Titan/PCH.h"
#include "Titan/Platform/OpenGL/OpenGLShader.h"
#include "Titan/Renderer/Renderer.h"

namespace Titan
{

    Shader* Shader::Create(const std::string& vertexSrc, const std::string& fragmentSrc)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None:
                TI_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
                return nullptr;
            case RendererAPI::API::OpenGL:
                return new OpenGLShader(vertexSrc, fragmentSrc);
        }

        TI_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    Shader* Shader::Create(const std::string& path)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None:
                TI_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
                return nullptr;
            case RendererAPI::API::OpenGL:
                return new OpenGLShader(path);
        }

        TI_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

} // namespace Titan
