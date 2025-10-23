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

        virtual void* GetColorAttachment() const override { return (void*)m_ColorAttachment; }

        virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

    private:
        uint32_t m_RendererID;
        uint32_t m_ColorAttachment, m_DepthAttachment;
        FramebufferSpecification m_Specification;
    };

} // namespace Titan