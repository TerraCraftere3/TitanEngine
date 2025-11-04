#include "ShaderStorageBuffer.h"
#include "Titan/PCH.h"
#include "Titan/Platform/OpenGL/OpenGLShaderStorageBuffer.h"
#include "Titan/Renderer/Renderer.h"

namespace Titan
{
    Ref<ShaderStorageBuffer> ShaderStorageBuffer::Create(uint32_t size, uint32_t binding)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None:
                TI_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
                return nullptr;
            case RendererAPI::API::OpenGL:
                return CreateRef<OpenGLShaderStorageBuffer>(size, binding);
        }

        TI_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
} // namespace Titan