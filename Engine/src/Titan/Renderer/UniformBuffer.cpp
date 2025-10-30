#include "UniformBuffer.h"
#include "Titan/PCH.h"

#include "Titan/Platform/OpenGL/OpenGLUniformBuffer.h"
#include "Titan/Renderer/Renderer.h"

namespace Titan
{

    Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
    {
        switch (Renderer::GetAPI())
        {
            case RendererAPI::API::None:
                TI_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
                return nullptr;
            case RendererAPI::API::OpenGL:
                return CreateRef<OpenGLUniformBuffer>(size, binding);
        }

        TI_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

} // namespace Titan