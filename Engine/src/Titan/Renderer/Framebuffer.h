#pragma once

#include "Titan/Core.h"

namespace Titan
{
    enum class FramebufferTextureFormat
    {
        None = 0,

        // Color formats - 8-bit
        RGBA8,
        RGB8,
        RG8,
        R8,

        // Color formats - 16-bit
        RGBA16,
        RGBA16F,
        RGB16F,
        RG16F,
        R16F,

        // Color formats - 32-bit
        RGBA32F,
        RGB32F,
        RG32F,
        R32F,

        // Integer formats
        RED_INTEGER,
        RG_INTEGER,
        RGB_INTEGER,
        RGBA_INTEGER,

        // 16-bit integer formats
        R16I,
        RG16I,
        RGB16I,
        RGBA16I,

        // 32-bit integer formats
        R32I,
        RG32I,
        RGB32I,
        RGBA32I,

        // Unsigned integer formats
        R8UI,
        RG8UI,
        RGB8UI,
        RGBA8UI,

        R16UI,
        RG16UI,
        RGB16UI,
        RGBA16UI,

        R32UI,
        RG32UI,
        RGB32UI,
        RGBA32UI,

        // sRGB formats
        SRGB8,
        SRGB8_ALPHA8,

        // Depth/stencil
        DEPTH24STENCIL8,
        DEPTH32F,
        DEPTH32F_STENCIL8,

        // Defaults
        Depth = DEPTH24STENCIL8
    };

    struct FramebufferTextureSpecification
    {
        FramebufferTextureSpecification() = default;
        FramebufferTextureSpecification(FramebufferTextureFormat format) : TextureFormat(format) {}

        FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None;
        // TODO: filtering/wrap
    };

    struct FramebufferAttachmentSpecification
    {
        FramebufferAttachmentSpecification() = default;
        FramebufferAttachmentSpecification(std::initializer_list<FramebufferTextureSpecification> attachments)
            : Attachments(attachments)
        {
        }

        std::vector<FramebufferTextureSpecification> Attachments;
    };

    struct FramebufferSpecification
    {
        uint32_t Width, Height;
        FramebufferAttachmentSpecification Attachments;
        uint32_t Samples = 1;

        bool SwapChainTarget = false;
    };

    class TI_API Framebuffer
    {
    public:
        virtual void Bind() = 0;
        virtual void Unbind() = 0;
        virtual void Resolve() = 0;
        virtual void Resize(uint32_t width, uint32_t height) = 0;
        virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;

        virtual void ClearAttachment(uint32_t attachmentIndex, int value) = 0;

        virtual void BindTexture(uint32_t attachmentIndex = 0, uint32_t bindIndex = 0) const = 0;
        virtual void BindDepthTexture(uint32_t bindIndex = 0) const = 0;
        virtual void* GetColorAttachment(uint32_t index = 0) const = 0;
        virtual void* GetDepthAttachment() const = 0;

        virtual const FramebufferSpecification& GetSpecification() const = 0;

        static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
    };

} // namespace Titan