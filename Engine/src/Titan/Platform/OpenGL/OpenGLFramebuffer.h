#pragma once

#include "Titan/Renderer/Framebuffer.h"

namespace Titan
{

    class OpenGLFramebuffer : public Framebuffer
    {
    public:
        OpenGLFramebuffer(const FramebufferSpecification& spec);
        virtual ~OpenGLFramebuffer();

        void Invalidate();

        virtual void Bind() override;
        virtual void Unbind() override;
        virtual void Resize(uint32_t width, uint32_t height) override;

        virtual void* GetDepthAttachment() const override { return (void*)(intptr_t)m_DepthAttachment; }
        virtual void* GetColorAttachment() const override { return (void*)(intptr_t)m_ColorAttachment; }

        virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

    private:
        uint32_t m_RendererID = 0;
        uint32_t m_ColorAttachment = 0, m_DepthAttachment = 0;
        FramebufferSpecification m_Specification;
    };

} // namespace Titan