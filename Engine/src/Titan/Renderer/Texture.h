#pragma once

#include <string>

#include "Titan/Core.h"

namespace Titan
{

    class TI_API Texture
    {
    public:
        virtual ~Texture() = default;

        virtual uint32_t GetWidth() const = 0;
        virtual uint32_t GetHeight() const = 0;
        virtual void* GetNativeTexture() const = 0;
        virtual std::string GetPath() const = 0;

        virtual void SetData(void* data, uint32_t size) = 0;

        virtual void Bind(uint32_t slot = 0) const = 0;

        virtual uint64_t GetBindlessHandle() = 0;
        virtual void MakeHandleResident() = 0;
        virtual void MakeHandleNonResident() = 0;
        virtual bool isValidBindlessHandle() = 0;

        virtual bool operator==(const Texture& other) const = 0;
    };

    enum TextureWrap
    {
        Repeat,
        MirroredRepeat,
        ClampToEdge,
        ClampToBorder
    };

    enum TextureFiltering
    {
        Nearest,
        MipmapNearest,
        Linear,
        MipmapLinear
    };

    struct TextureSettings
    {
        TextureWrap HorizontalWrap = TextureWrap::Repeat;
        TextureWrap VerticalWrap = TextureWrap::Repeat;
        TextureFiltering MinFilter = TextureFiltering::MipmapLinear;
        TextureFiltering MagFilter = TextureFiltering::Linear;

        TextureSettings() = default;
    };

    class TI_API Texture2D : public Texture
    {
    public:
        static Ref<Texture2D> Create(uint32_t width, uint32_t height);
        static Ref<Texture2D> Create(const std::string& path, TextureSettings settings = TextureSettings());
    };

    namespace Utils
    {
        inline TextureWrap StringToTextureWrap(const std::string& str)
        {
            if (str == "Repeat")
                return TextureWrap::Repeat;
            if (str == "MirroredRepeat")
                return TextureWrap::MirroredRepeat;
            if (str == "ClampToEdge")
                return TextureWrap::ClampToEdge;
            if (str == "ClampToBorder")
                return TextureWrap::ClampToBorder;
            return TextureWrap::Repeat; // default fallback
        }

        inline std::string TextureWrapToString(TextureWrap wrap)
        {
            switch (wrap)
            {
                case TextureWrap::Repeat:
                    return "Repeat";
                case TextureWrap::MirroredRepeat:
                    return "MirroredRepeat";
                case TextureWrap::ClampToEdge:
                    return "ClampToEdge";
                case TextureWrap::ClampToBorder:
                    return "ClampToBorder";
                default:
                    return "Repeat";
            }
        }

        // ------------------- TextureFiltering -------------------
        inline TextureFiltering StringToTextureFiltering(const std::string& str)
        {
            if (str == "Nearest")
                return TextureFiltering::Nearest;
            if (str == "MipmapNearest")
                return TextureFiltering::MipmapNearest;
            if (str == "Linear")
                return TextureFiltering::Linear;
            if (str == "MipmapLinear")
                return TextureFiltering::MipmapLinear;
            return TextureFiltering::Linear; // default fallback
        }

        inline std::string TextureFilteringToString(TextureFiltering filter)
        {
            switch (filter)
            {
                case TextureFiltering::Nearest:
                    return "Nearest";
                case TextureFiltering::MipmapNearest:
                    return "MipmapNearest";
                case TextureFiltering::Linear:
                    return "Linear";
                case TextureFiltering::MipmapLinear:
                    return "MipmapLinear";
                default:
                    return "Linear";
            }
        }
    } // namespace Utils

} // namespace Titan