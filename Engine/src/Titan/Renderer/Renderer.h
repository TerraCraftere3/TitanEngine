#pragma once

#include "RHI/RendererAPI.h"
#include "Titan/PCH.h"

namespace Titan
{

    class TI_API Renderer
    {
    public:
        static void OnWindowResize(uint32_t width, uint32_t height);
        static void Init();
        static void Shutdown();

        inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
    };

} // namespace Titan