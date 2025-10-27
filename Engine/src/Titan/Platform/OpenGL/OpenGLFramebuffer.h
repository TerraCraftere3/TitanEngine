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
        virtual void Resolve() override;
        virtual void Resize(uint32_t width, uint32_t height) override;
        virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) override;

        virtual void ClearAttachment(uint32_t attachmentIndex, int value) override;

        virtual void* GetDepthAttachment() const override { return (void*)(intptr_t)m_DepthAttachment; }
        virtual void* GetColorAttachment(uint32_t index = 0) const override
        {
            TI_CORE_ASSERT(index < m_ColorAttachments.size());

            if (m_Specification.Samples > 1 && !m_ResolvedColorAttachments.empty())
                return (void*)(intptr_t)m_ResolvedColorAttachments[index];

            return (void*)(intptr_t)m_ColorAttachments[index];
        }

        virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

    private:
        uint32_t m_RendererID = 0;
        FramebufferSpecification m_Specification;

        std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecifications;
        FramebufferTextureSpecification m_DepthAttachmentSpecification = FramebufferTextureFormat::None;

        std::vector<uint32_t> m_ColorAttachments;
        uint32_t m_DepthAttachment = 0;

        uint32_t m_ResolvedRendererID = 0;
        std::vector<uint32_t> m_ResolvedColorAttachments;
    };

} // namespace Titan