#pragma once

#include "Texture.h"
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

    inline static const char* FramebufferTextureFormatToString(FramebufferTextureFormat format)
    {
        switch (format)
        {
            case FramebufferTextureFormat::None:
                return "None";

            // 8-bit
            case FramebufferTextureFormat::RGBA8:
                return "RGBA8";
            case FramebufferTextureFormat::RGB8:
                return "RGB8";
            case FramebufferTextureFormat::RG8:
                return "RG8";
            case FramebufferTextureFormat::R8:
                return "R8";

            // 16-bit
            case FramebufferTextureFormat::RGBA16:
                return "RGBA16";
            case FramebufferTextureFormat::RGBA16F:
                return "RGBA16F";
            case FramebufferTextureFormat::RGB16F:
                return "RGB16F";
            case FramebufferTextureFormat::RG16F:
                return "RG16F";
            case FramebufferTextureFormat::R16F:
                return "R16F";

            // 32-bit
            case FramebufferTextureFormat::RGBA32F:
                return "RGBA32F";
            case FramebufferTextureFormat::RGB32F:
                return "RGB32F";
            case FramebufferTextureFormat::RG32F:
                return "RG32F";
            case FramebufferTextureFormat::R32F:
                return "R32F";

            // Integer formats
            case FramebufferTextureFormat::RED_INTEGER:
                return "RED_INTEGER";
            case FramebufferTextureFormat::RG_INTEGER:
                return "RG_INTEGER";
            case FramebufferTextureFormat::RGB_INTEGER:
                return "RGB_INTEGER";
            case FramebufferTextureFormat::RGBA_INTEGER:
                return "RGBA_INTEGER";

            // 16-bit integer formats
            case FramebufferTextureFormat::R16I:
                return "R16I";
            case FramebufferTextureFormat::RG16I:
                return "RG16I";
            case FramebufferTextureFormat::RGB16I:
                return "RGB16I";
            case FramebufferTextureFormat::RGBA16I:
                return "RGBA16I";

            // 32-bit integer formats
            case FramebufferTextureFormat::R32I:
                return "R32I";
            case FramebufferTextureFormat::RG32I:
                return "RG32I";
            case FramebufferTextureFormat::RGB32I:
                return "RGB32I";
            case FramebufferTextureFormat::RGBA32I:
                return "RGBA32I";

            // Unsigned integer formats
            case FramebufferTextureFormat::R8UI:
                return "R8UI";
            case FramebufferTextureFormat::RG8UI:
                return "RG8UI";
            case FramebufferTextureFormat::RGB8UI:
                return "RGB8UI";
            case FramebufferTextureFormat::RGBA8UI:
                return "RGBA8UI";

            case FramebufferTextureFormat::R16UI:
                return "R16UI";
            case FramebufferTextureFormat::RG16UI:
                return "RG16UI";
            case FramebufferTextureFormat::RGB16UI:
                return "RGB16UI";
            case FramebufferTextureFormat::RGBA16UI:
                return "RGBA16UI";

            case FramebufferTextureFormat::R32UI:
                return "R32UI";
            case FramebufferTextureFormat::RG32UI:
                return "RG32UI";
            case FramebufferTextureFormat::RGB32UI:
                return "RGB32UI";
            case FramebufferTextureFormat::RGBA32UI:
                return "RGBA32UI";

            // sRGB formats
            case FramebufferTextureFormat::SRGB8:
                return "SRGB8";
            case FramebufferTextureFormat::SRGB8_ALPHA8:
                return "SRGB8_ALPHA8";

            // Depth / Stencil
            case FramebufferTextureFormat::DEPTH24STENCIL8:
                return "DEPTH24STENCIL8";
            case FramebufferTextureFormat::DEPTH32F:
                return "DEPTH32F";
            case FramebufferTextureFormat::DEPTH32F_STENCIL8:
                return "DEPTH32F_STENCIL8";
        }

        return "Unknown";
    }

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
        bool SwapChainTarget = false;
    };

    class TI_API Framebuffer
    {
    public:
        virtual void Resolve() = 0;
        virtual void Resize(uint32_t width, uint32_t height) = 0;
        virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;

        virtual uint32_t GetWidth() = 0;
        virtual uint32_t GetHeight() = 0;

        virtual void ClearAttachment(uint32_t attachmentIndex, int value) = 0;

        // New: expose attachments as Texture2D for consistent usage throughout the engine
        virtual Ref<Texture2D> GetColorAttachmentTexture(uint32_t index = 0) const = 0;
        virtual Ref<Texture2D> GetDepthAttachmentTexture() const = 0;

        virtual const FramebufferSpecification& GetSpecification() const = 0;

        static Ref<Framebuffer> Create(const FramebufferSpecification& spec);
    };

} // namespace Titan