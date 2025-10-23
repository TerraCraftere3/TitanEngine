#pragma once

#include "Titan/Core.h"

namespace Titan
{

    struct FramebufferSpecification
    {
        uint32_t Width, Height;
        uint32_t Samples = 1;

        bool SwapChainTarget = false;
    };

    class TI_API Framebuffer
    {
    public:
        virtual void Bind() = 0;
        virtual void Unbind() = 0;

        virtual void* GetColorAttachment() const = 0;

        virtual const FramebufferSpecification& GetSpecification() const = 0;

        static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
    };

} // namespace Titan