#include "Titan/Platform/OpenGL/OpenGLFramebuffer.h"
#include "OpenGLFramebuffer.h"
#include "Titan/PCH.h"

namespace Titan
{

    static const uint32_t s_MaxFramebufferSize = 8192;

    namespace Utils
    {

        static GLenum TextureTarget(bool multisampled)
        {
            return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
        }

        static void CreateTextures(bool multisampled, uint32_t* outID, uint32_t count)
        {
            glCreateTextures(TextureTarget(multisampled), count, outID);
        }

        static void BindTexture(bool multisampled, uint32_t id)
        {
            glBindTexture(TextureTarget(multisampled), id);
        }

        static void AttachColorTexture(uint32_t id, int samples, GLenum internalFormat, GLenum format, GLenum type,
                                       uint32_t width, uint32_t height, int index)
        {
            bool multisampled = samples > 1;
            if (multisampled)
            {
                glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_FALSE);
            }
            else
            {
                glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, nullptr);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(multisampled), id, 0);
        }

        static void AttachDepthTexture(uint32_t id, int samples, GLenum format, GLenum attachmentType, uint32_t width,
                                       uint32_t height)
        {
            bool multisampled = samples > 1;
            if (multisampled)
            {
                glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
            }
            else
            {
                glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }

            glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, TextureTarget(multisampled), id, 0);
        }

        static bool IsDepthFormat(FramebufferTextureFormat format)
        {
            switch (format)
            {
                case FramebufferTextureFormat::DEPTH24STENCIL8:
                case FramebufferTextureFormat::DEPTH32F:
                case FramebufferTextureFormat::DEPTH32F_STENCIL8:
                    return true;
            }

            return false;
        }

        static GLenum TitanFBTextureFormatToGL(FramebufferTextureFormat format)
        {
            switch (format)
            {
                // 8-bit formats
                case FramebufferTextureFormat::RGBA8:
                    return GL_RGBA8;
                case FramebufferTextureFormat::RGB8:
                    return GL_RGB8;
                case FramebufferTextureFormat::RG8:
                    return GL_RG8;
                case FramebufferTextureFormat::R8:
                    return GL_R8;

                // 16-bit formats
                case FramebufferTextureFormat::RGBA16:
                    return GL_RGBA16;
                case FramebufferTextureFormat::RGBA16F:
                    return GL_RGBA16F;
                case FramebufferTextureFormat::RGB16F:
                    return GL_RGB16F;
                case FramebufferTextureFormat::RG16F:
                    return GL_RG16F;
                case FramebufferTextureFormat::R16F:
                    return GL_R16F;

                // 32-bit float formats
                case FramebufferTextureFormat::RGBA32F:
                    return GL_RGBA32F;
                case FramebufferTextureFormat::RGB32F:
                    return GL_RGB32F;
                case FramebufferTextureFormat::RG32F:
                    return GL_RG32F;
                case FramebufferTextureFormat::R32F:
                    return GL_R32F;

                // Integer formats (legacy)
                case FramebufferTextureFormat::RED_INTEGER:
                    return GL_R32I;
                case FramebufferTextureFormat::RG_INTEGER:
                    return GL_RG32I;
                case FramebufferTextureFormat::RGB_INTEGER:
                    return GL_RGB32I;
                case FramebufferTextureFormat::RGBA_INTEGER:
                    return GL_RGBA32I;

                // 16-bit signed integer
                case FramebufferTextureFormat::R16I:
                    return GL_R16I;
                case FramebufferTextureFormat::RG16I:
                    return GL_RG16I;
                case FramebufferTextureFormat::RGB16I:
                    return GL_RGB16I;
                case FramebufferTextureFormat::RGBA16I:
                    return GL_RGBA16I;

                // 32-bit signed integer
                case FramebufferTextureFormat::R32I:
                    return GL_R32I;
                case FramebufferTextureFormat::RG32I:
                    return GL_RG32I;
                case FramebufferTextureFormat::RGB32I:
                    return GL_RGB32I;
                case FramebufferTextureFormat::RGBA32I:
                    return GL_RGBA32I;

                // 8-bit unsigned integer
                case FramebufferTextureFormat::R8UI:
                    return GL_R8UI;
                case FramebufferTextureFormat::RG8UI:
                    return GL_RG8UI;
                case FramebufferTextureFormat::RGB8UI:
                    return GL_RGB8UI;
                case FramebufferTextureFormat::RGBA8UI:
                    return GL_RGBA8UI;

                // 16-bit unsigned integer
                case FramebufferTextureFormat::R16UI:
                    return GL_R16UI;
                case FramebufferTextureFormat::RG16UI:
                    return GL_RG16UI;
                case FramebufferTextureFormat::RGB16UI:
                    return GL_RGB16UI;
                case FramebufferTextureFormat::RGBA16UI:
                    return GL_RGBA16UI;

                // 32-bit unsigned integer
                case FramebufferTextureFormat::R32UI:
                    return GL_R32UI;
                case FramebufferTextureFormat::RG32UI:
                    return GL_RG32UI;
                case FramebufferTextureFormat::RGB32UI:
                    return GL_RGB32UI;
                case FramebufferTextureFormat::RGBA32UI:
                    return GL_RGBA32UI;

                // sRGB formats
                case FramebufferTextureFormat::SRGB8:
                    return GL_SRGB8;
                case FramebufferTextureFormat::SRGB8_ALPHA8:
                    return GL_SRGB8_ALPHA8;
            }

            TI_CORE_ASSERT(false);
            return 0;
        }

        struct FormatInfo
        {
            GLenum internalFormat;
            GLenum format;
            GLenum type;
        };

        static FormatInfo GetFormatInfo(FramebufferTextureFormat format)
        {
            switch (format)
            {
                // 8-bit formats
                case FramebufferTextureFormat::RGBA8:
                    return {GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE};
                case FramebufferTextureFormat::RGB8:
                    return {GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE};
                case FramebufferTextureFormat::RG8:
                    return {GL_RG8, GL_RG, GL_UNSIGNED_BYTE};
                case FramebufferTextureFormat::R8:
                    return {GL_R8, GL_RED, GL_UNSIGNED_BYTE};

                // 16-bit formats
                case FramebufferTextureFormat::RGBA16:
                    return {GL_RGBA16, GL_RGBA, GL_UNSIGNED_SHORT};
                case FramebufferTextureFormat::RGBA16F:
                    return {GL_RGBA16F, GL_RGBA, GL_FLOAT};
                case FramebufferTextureFormat::RGB16F:
                    return {GL_RGB16F, GL_RGB, GL_FLOAT};
                case FramebufferTextureFormat::RG16F:
                    return {GL_RG16F, GL_RG, GL_FLOAT};
                case FramebufferTextureFormat::R16F:
                    return {GL_R16F, GL_RED, GL_FLOAT};

                // 32-bit float formats
                case FramebufferTextureFormat::RGBA32F:
                    return {GL_RGBA32F, GL_RGBA, GL_FLOAT};
                case FramebufferTextureFormat::RGB32F:
                    return {GL_RGB32F, GL_RGB, GL_FLOAT};
                case FramebufferTextureFormat::RG32F:
                    return {GL_RG32F, GL_RG, GL_FLOAT};
                case FramebufferTextureFormat::R32F:
                    return {GL_R32F, GL_RED, GL_FLOAT};

                // Integer formats (legacy)
                case FramebufferTextureFormat::RED_INTEGER:
                    return {GL_R32I, GL_RED_INTEGER, GL_INT};
                case FramebufferTextureFormat::RG_INTEGER:
                    return {GL_RG32I, GL_RG_INTEGER, GL_INT};
                case FramebufferTextureFormat::RGB_INTEGER:
                    return {GL_RGB32I, GL_RGB_INTEGER, GL_INT};
                case FramebufferTextureFormat::RGBA_INTEGER:
                    return {GL_RGBA32I, GL_RGBA_INTEGER, GL_INT};

                // 16-bit signed integer
                case FramebufferTextureFormat::R16I:
                    return {GL_R16I, GL_RED_INTEGER, GL_SHORT};
                case FramebufferTextureFormat::RG16I:
                    return {GL_RG16I, GL_RG_INTEGER, GL_SHORT};
                case FramebufferTextureFormat::RGB16I:
                    return {GL_RGB16I, GL_RGB_INTEGER, GL_SHORT};
                case FramebufferTextureFormat::RGBA16I:
                    return {GL_RGBA16I, GL_RGBA_INTEGER, GL_SHORT};

                // 32-bit signed integer
                case FramebufferTextureFormat::R32I:
                    return {GL_R32I, GL_RED_INTEGER, GL_INT};
                case FramebufferTextureFormat::RG32I:
                    return {GL_RG32I, GL_RG_INTEGER, GL_INT};
                case FramebufferTextureFormat::RGB32I:
                    return {GL_RGB32I, GL_RGB_INTEGER, GL_INT};
                case FramebufferTextureFormat::RGBA32I:
                    return {GL_RGBA32I, GL_RGBA_INTEGER, GL_INT};

                // 8-bit unsigned integer
                case FramebufferTextureFormat::R8UI:
                    return {GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE};
                case FramebufferTextureFormat::RG8UI:
                    return {GL_RG8UI, GL_RG_INTEGER, GL_UNSIGNED_BYTE};
                case FramebufferTextureFormat::RGB8UI:
                    return {GL_RGB8UI, GL_RGB_INTEGER, GL_UNSIGNED_BYTE};
                case FramebufferTextureFormat::RGBA8UI:
                    return {GL_RGBA8UI, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE};

                // 16-bit unsigned integer
                case FramebufferTextureFormat::R16UI:
                    return {GL_R16UI, GL_RED_INTEGER, GL_UNSIGNED_SHORT};
                case FramebufferTextureFormat::RG16UI:
                    return {GL_RG16UI, GL_RG_INTEGER, GL_UNSIGNED_SHORT};
                case FramebufferTextureFormat::RGB16UI:
                    return {GL_RGB16UI, GL_RGB_INTEGER, GL_UNSIGNED_SHORT};
                case FramebufferTextureFormat::RGBA16UI:
                    return {GL_RGBA16UI, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT};

                // 32-bit unsigned integer
                case FramebufferTextureFormat::R32UI:
                    return {GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT};
                case FramebufferTextureFormat::RG32UI:
                    return {GL_RG32UI, GL_RG_INTEGER, GL_UNSIGNED_INT};
                case FramebufferTextureFormat::RGB32UI:
                    return {GL_RGB32UI, GL_RGB_INTEGER, GL_UNSIGNED_INT};
                case FramebufferTextureFormat::RGBA32UI:
                    return {GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT};

                // sRGB formats
                case FramebufferTextureFormat::SRGB8:
                    return {GL_SRGB8, GL_RGB, GL_UNSIGNED_BYTE};
                case FramebufferTextureFormat::SRGB8_ALPHA8:
                    return {GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE};
            }

            TI_CORE_ASSERT(false);
            return {0, 0, 0};
        }

        static bool IsIntegerFormat(FramebufferTextureFormat format)
        {
            switch (format)
            {
                case FramebufferTextureFormat::RED_INTEGER:
                case FramebufferTextureFormat::RG_INTEGER:
                case FramebufferTextureFormat::RGB_INTEGER:
                case FramebufferTextureFormat::RGBA_INTEGER:
                case FramebufferTextureFormat::R16I:
                case FramebufferTextureFormat::RG16I:
                case FramebufferTextureFormat::RGB16I:
                case FramebufferTextureFormat::RGBA16I:
                case FramebufferTextureFormat::R32I:
                case FramebufferTextureFormat::RG32I:
                case FramebufferTextureFormat::RGB32I:
                case FramebufferTextureFormat::RGBA32I:
                case FramebufferTextureFormat::R8UI:
                case FramebufferTextureFormat::RG8UI:
                case FramebufferTextureFormat::RGB8UI:
                case FramebufferTextureFormat::RGBA8UI:
                case FramebufferTextureFormat::R16UI:
                case FramebufferTextureFormat::RG16UI:
                case FramebufferTextureFormat::RGB16UI:
                case FramebufferTextureFormat::RGBA16UI:
                case FramebufferTextureFormat::R32UI:
                case FramebufferTextureFormat::RG32UI:
                case FramebufferTextureFormat::RGB32UI:
                case FramebufferTextureFormat::RGBA32UI:
                    return true;
            }
            return false;
        }

    } // namespace Utils

    OpenGLFramebuffer::OpenGLFramebuffer(const FramebufferSpecification& spec) : m_Specification(spec)
    {
        for (auto spec : m_Specification.Attachments.Attachments)
        {
            if (!Utils::IsDepthFormat(spec.TextureFormat))
                m_ColorAttachmentSpecifications.emplace_back(spec);
            else
                m_DepthAttachmentSpecification = spec;
        }

        Invalidate();
    }

    OpenGLFramebuffer::~OpenGLFramebuffer()
    {
        glDeleteFramebuffers(1, &m_RendererID);
        glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
        glDeleteTextures(1, &m_DepthAttachment);
    }

    void OpenGLFramebuffer::Invalidate()
    {
        if (m_RendererID)
        {
            glDeleteFramebuffers(1, &m_RendererID);
            glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
            glDeleteTextures(1, &m_DepthAttachment);

            if (m_ResolvedRendererID)
            {
                glDeleteFramebuffers(1, &m_ResolvedRendererID);
                glDeleteTextures(m_ResolvedColorAttachments.size(), m_ResolvedColorAttachments.data());
            }

            m_ColorAttachments.clear();
            m_ResolvedColorAttachments.clear();
            m_DepthAttachment = 0;
        }

        glCreateFramebuffers(1, &m_RendererID);
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

        bool multisample = m_Specification.Samples > 1;

        // Attachments
        if (m_ColorAttachmentSpecifications.size())
        {
            m_ColorAttachments.resize(m_ColorAttachmentSpecifications.size());
            Utils::CreateTextures(multisample, m_ColorAttachments.data(), m_ColorAttachments.size());

            for (size_t i = 0; i < m_ColorAttachments.size(); i++)
            {
                Utils::BindTexture(multisample, m_ColorAttachments[i]);
                auto formatInfo = Utils::GetFormatInfo(m_ColorAttachmentSpecifications[i].TextureFormat);
                Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, formatInfo.internalFormat,
                                          formatInfo.format, formatInfo.type, m_Specification.Width,
                                          m_Specification.Height, i);
            }
        }

        if (m_DepthAttachmentSpecification.TextureFormat != FramebufferTextureFormat::None)
        {
            Utils::CreateTextures(multisample, &m_DepthAttachment, 1);
            Utils::BindTexture(multisample, m_DepthAttachment);
            switch (m_DepthAttachmentSpecification.TextureFormat)
            {
                case FramebufferTextureFormat::DEPTH24STENCIL8:
                    Utils::AttachDepthTexture(m_DepthAttachment, m_Specification.Samples, GL_DEPTH24_STENCIL8,
                                              GL_DEPTH_STENCIL_ATTACHMENT, m_Specification.Width,
                                              m_Specification.Height);
                    break;
                case FramebufferTextureFormat::DEPTH32F:
                    Utils::AttachDepthTexture(m_DepthAttachment, m_Specification.Samples, GL_DEPTH_COMPONENT32F,
                                              GL_DEPTH_ATTACHMENT, m_Specification.Width, m_Specification.Height);
                    break;
                case FramebufferTextureFormat::DEPTH32F_STENCIL8:
                    Utils::AttachDepthTexture(m_DepthAttachment, m_Specification.Samples, GL_DEPTH32F_STENCIL8,
                                              GL_DEPTH_STENCIL_ATTACHMENT, m_Specification.Width,
                                              m_Specification.Height);
                    break;
            }
        }

        if (m_ColorAttachments.size() > 1)
        {
            TI_CORE_ASSERT(m_ColorAttachments.size() <= 4);
            GLenum buffers[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
                                 GL_COLOR_ATTACHMENT3};
            glDrawBuffers(m_ColorAttachments.size(), buffers);
        }
        else if (m_ColorAttachments.empty())
        {
            glDrawBuffer(GL_NONE);
        }

        TI_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
                       "Framebuffer is incomplete!");

        if (multisample && m_ColorAttachmentSpecifications.size())
        {
            glCreateFramebuffers(1, &m_ResolvedRendererID);
            glBindFramebuffer(GL_FRAMEBUFFER, m_ResolvedRendererID);

            m_ResolvedColorAttachments.resize(m_ColorAttachmentSpecifications.size());
            Utils::CreateTextures(false, m_ResolvedColorAttachments.data(), m_ResolvedColorAttachments.size());

            for (size_t i = 0; i < m_ResolvedColorAttachments.size(); i++)
            {
                Utils::BindTexture(false, m_ResolvedColorAttachments[i]);
                auto formatInfo = Utils::GetFormatInfo(m_ColorAttachmentSpecifications[i].TextureFormat);
                Utils::AttachColorTexture(m_ResolvedColorAttachments[i], 1, formatInfo.internalFormat,
                                          formatInfo.format, formatInfo.type, m_Specification.Width,
                                          m_Specification.Height, i);
            }

            if (m_ResolvedColorAttachments.size() > 1)
            {
                GLenum buffers[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
                                     GL_COLOR_ATTACHMENT3};
                glDrawBuffers(m_ResolvedColorAttachments.size(), buffers);
            }

            TI_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE,
                           "Resolved framebuffer is incomplete!");
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenGLFramebuffer::Bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
        glViewport(0, 0, m_Specification.Width, m_Specification.Height);
    }

    void OpenGLFramebuffer::Unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenGLFramebuffer::Resolve()
    {
        if (m_Specification.Samples <= 1 || !m_ResolvedRendererID)
            return;

        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_RendererID);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_ResolvedRendererID);

        for (size_t i = 0; i < m_ColorAttachments.size(); i++)
        {
            auto& spec = m_ColorAttachmentSpecifications[i];

            // Only blit non-integer textures
            if (Utils::IsIntegerFormat(spec.TextureFormat))
                continue;

            glReadBuffer(GL_COLOR_ATTACHMENT0 + i);
            glDrawBuffer(GL_COLOR_ATTACHMENT0 + i);

            glBlitFramebuffer(0, 0, m_Specification.Width, m_Specification.Height, 0, 0, m_Specification.Width,
                              m_Specification.Height, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void OpenGLFramebuffer::Resize(uint32_t width, uint32_t height)
    {
        if (width == 0 || height == 0 || width > s_MaxFramebufferSize || height > s_MaxFramebufferSize)
        {
            TI_CORE_WARN("Attempted to rezize framebuffer to {0}, {1}", width, height);
            return;
        }
        m_Specification.Width = width;
        m_Specification.Height = height;

        Invalidate();
    }

    int OpenGLFramebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y)
    {
        TI_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size());

        auto& spec = m_ColorAttachmentSpecifications[attachmentIndex];

        int pixelData = 0;

        if (m_Specification.Samples > 1)
        {
            // For integer attachments, read manually
            if (Utils::IsIntegerFormat(spec.TextureFormat))
            {
                glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
                glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);

                // Cannot blit multisample integer, must read via glReadPixels
                glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);
            }
            else
            {
                // For non-integer formats, blit to resolved FBO
                Resolve();

                glBindFramebuffer(GL_FRAMEBUFFER, m_ResolvedRendererID);
                glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
                unsigned char data[4];
                glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
                pixelData = data[0]; // return red channel
            }

            glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
            return pixelData;
        }

        // Single-sample
        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
        glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);

        if (Utils::IsIntegerFormat(spec.TextureFormat))
            glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);
        else
        {
            unsigned char data[4];
            glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
            pixelData = data[0]; // red channel
        }

        return pixelData;
    }

    void OpenGLFramebuffer::ClearAttachment(uint32_t attachmentIndex, int value)
    {
        TI_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size());

        auto& spec = m_ColorAttachmentSpecifications[attachmentIndex];

        glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

        if (Utils::IsIntegerFormat(spec.TextureFormat))
        {
            // Use clear buffer for integer textures
            glClearBufferiv(GL_COLOR, attachmentIndex, &value);
        }
        else
        {
            // Use glClearTexImage for non-integer textures
            glClearTexImage(m_ColorAttachments[attachmentIndex], 0, Utils::TitanFBTextureFormatToGL(spec.TextureFormat),
                            GL_UNSIGNED_BYTE, nullptr);
        }
    }

} // namespace Titan