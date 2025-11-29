#pragma once

#include "Titan/Renderer/Framebuffer.h"
// clang-format off
#ifdef APIENTRY
    #undef APIENTRY
#endif
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on
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

        virtual uint32_t GetWidth() override { return m_Specification.Width; };
        virtual uint32_t GetHeight() override { return m_Specification.Height; };

        virtual void ClearAttachment(uint32_t attachmentIndex, int value) override;

        virtual Ref<Texture2D> GetDepthAttachmentTexture() const override { return m_DepthAttachmentTex; }
        virtual Ref<Texture2D> GetColorAttachmentTexture(uint32_t index = 0) const override
        {
            TI_CORE_ASSERT(index < m_ColorAttachmentTex.size());
            return m_ColorAttachmentTex[index];
        }

        virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

    private:
        uint32_t m_RendererID = 0;
        FramebufferSpecification m_Specification;

        std::vector<FramebufferTextureSpecification> m_ColorAttachmentSpecifications;
        FramebufferTextureSpecification m_DepthAttachmentSpecification = FramebufferTextureFormat::None;

        std::vector<uint32_t> m_ColorAttachments; // GL IDs (internal)
        uint32_t m_DepthAttachment = 0;           // GL ID (internal)

        std::vector<Ref<Texture2D>> m_ColorAttachmentTex;
        Ref<Texture2D> m_DepthAttachmentTex;
    };

} // namespace Titan