#include "OpenGLTexture.h"
#include "Titan/PCH.h"
#include "Titan/Utils/PlatformUtils.h"
#include "nanosvg.h"
#include "nanosvgrast.h"
#include "stb_image.h"
// clang-format off
#ifdef APIENTRY
    #undef APIENTRY
#endif
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on
namespace Titan
{
    OpenGLTexture2D::OpenGLTexture2D(void* ptr, const std::string& path) : m_Path(path)
    {
        m_RendererID = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(ptr));
    }

    OpenGLTexture2D::OpenGLTexture2D(const std::string& path, TextureSettings settings) : m_Path(path)
    {
        int width = 0, height = 0, channels = 4;
        unsigned char* data = nullptr;

        // Check file extension
        auto ext = path.substr(path.find_last_of(".") + 1);
        for (auto& c : ext)
            c = std::tolower(c);
        if (ext == "svg")
        {
            width = height = 256;
            data = new unsigned char[width * height * 4]; // RGBA

            NSVGimage* image = nsvgParseFromFile(path.c_str(), "px", 96);
            TI_CORE_ASSERT(image, "Failed to load SVG!");

            NSVGrasterizer* rast = nsvgCreateRasterizer();

            float scale = float(width) / image->width;
            nsvgRasterize(rast, image, 0, 0, scale, data, width, height, width * 4);

            // --- Flip vertically ---
            for (int y = 0; y < height / 2; y++)
            {
                int opposite = height - y - 1;
                for (int x = 0; x < width * 4; x++)
                    std::swap(data[y * width * 4 + x], data[opposite * width * 4 + x]);
            }

            nsvgDeleteRasterizer(rast);
            nsvgDelete(image);

            m_InternalFormat = GL_RGBA8;
            m_DataFormat = GL_RGBA;
        }

        else
        {
            stbi_set_flip_vertically_on_load(1);
            data = stbi_load(path.c_str(), &width, &height, &channels, 0);
            TI_CORE_ASSERT(data, "Failed to load image!");

            if (channels == 4)
            {
                m_InternalFormat = GL_RGBA8;
                m_DataFormat = GL_RGBA;
            }
            else if (channels == 3)
            {
                m_InternalFormat = GL_RGB8;
                m_DataFormat = GL_RGB;
            }
            else if (channels == 2)
            {
                m_InternalFormat = GL_RG8;
                m_DataFormat = GL_RG;
            }
            else if (channels == 1)
            {
                m_InternalFormat = GL_R8;
                m_DataFormat = GL_RED;
            }

            TI_CORE_ASSERT(m_InternalFormat & m_DataFormat, "Format not supported!");
        }

        m_Width = width;
        m_Height = height;

        glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
        glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

        auto wrapToGL = [](TextureWrap wrap) -> GLenum
        {
            switch (wrap)
            {
                case TextureWrap::Repeat:
                    return GL_REPEAT;
                case TextureWrap::MirroredRepeat:
                    return GL_MIRRORED_REPEAT;
                case TextureWrap::ClampToEdge:
                    return GL_CLAMP_TO_EDGE;
                case TextureWrap::ClampToBorder:
                    return GL_CLAMP_TO_BORDER;
                default:
                    return GL_REPEAT;
            }
        };

        auto filterToGL = [](TextureFiltering filter) -> GLenum
        {
            switch (filter)
            {
                case TextureFiltering::Nearest:
                    return GL_NEAREST;
                case TextureFiltering::MipmapNearest:
                    return GL_NEAREST_MIPMAP_NEAREST;
                case TextureFiltering::Linear:
                    return GL_LINEAR;
                case TextureFiltering::MipmapLinear:
                    return GL_LINEAR_MIPMAP_LINEAR;
                default:
                    return GL_LINEAR;
            }
        };

        glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, filterToGL(settings.MinFilter));
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, filterToGL(settings.MagFilter));
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, wrapToGL(settings.HorizontalWrap));
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, wrapToGL(settings.VerticalWrap));

        if (settings.MinFilter == TextureFiltering::MipmapNearest ||
            settings.MinFilter == TextureFiltering::MipmapLinear)
        {
            glGenerateTextureMipmap(m_RendererID);
        }

        glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);

        if (ext == "svg")
            delete[] data;
        else
            stbi_image_free(data);
    }

    OpenGLTexture2D::OpenGLTexture2D(uint32_t width, uint32_t height, TextureFormat format, TextureSettings settings)
        : m_Width(width), m_Height(height), m_Path("[internal]")
    {
        // -----------------------------
        // Convert TextureFormat → GL formats
        // -----------------------------
        switch (format)
        {
            case TextureFormat::RGBA8:
                m_InternalFormat = GL_RGBA8;
                m_DataFormat = GL_RGBA;
                break;
            case TextureFormat::RGB8:
                m_InternalFormat = GL_RGB8;
                m_DataFormat = GL_RGB;
                break;
            case TextureFormat::RG8:
                m_InternalFormat = GL_RG8;
                m_DataFormat = GL_RG;
                break;
            case TextureFormat::RED8:
                m_InternalFormat = GL_R8;
                m_DataFormat = GL_RED;
                break;

            case TextureFormat::RGBA16F:
                m_InternalFormat = GL_RGBA16F;
                m_DataFormat = GL_RGBA;
                break;
            case TextureFormat::RGB16F:
                m_InternalFormat = GL_RGB16F;
                m_DataFormat = GL_RGB;
                break;
            case TextureFormat::RG16F:
                m_InternalFormat = GL_RG16F;
                m_DataFormat = GL_RG;
                break;
            case TextureFormat::RED16F:
                m_InternalFormat = GL_R16F;
                m_DataFormat = GL_RED;
                break;

            case TextureFormat::RGBA32F:
                m_InternalFormat = GL_RGBA32F;
                m_DataFormat = GL_RGBA;
                break;
            case TextureFormat::RGB32F:
                m_InternalFormat = GL_RGB32F;
                m_DataFormat = GL_RGB;
                break;
            case TextureFormat::RG32F:
                m_InternalFormat = GL_RG32F;
                m_DataFormat = GL_RG;
                break;

            default:
                TI_CORE_ASSERT(false, "Unsupported internal texture format!");
        }

        // Create texture object + storage
        glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
        glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

        // -----------------------------
        // Apply wrap settings
        // -----------------------------
        auto wrapToGL = [](TextureWrap w) -> GLenum
        {
            switch (w)
            {
                case TextureWrap::Repeat:
                    return GL_REPEAT;
                case TextureWrap::MirroredRepeat:
                    return GL_MIRRORED_REPEAT;
                case TextureWrap::ClampToEdge:
                    return GL_CLAMP_TO_EDGE;
                case TextureWrap::ClampToBorder:
                    return GL_CLAMP_TO_BORDER;
                default:
                    return GL_REPEAT;
            }
        };

        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, wrapToGL(settings.HorizontalWrap));
        glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, wrapToGL(settings.VerticalWrap));

        // -----------------------------
        // Apply filtering settings
        // -----------------------------
        auto filterToGL = [](TextureFiltering f) -> GLenum
        {
            switch (f)
            {
                case TextureFiltering::Nearest:
                    return GL_NEAREST;
                case TextureFiltering::Linear:
                    return GL_LINEAR;
                case TextureFiltering::MipmapNearest:
                    return GL_NEAREST_MIPMAP_NEAREST;
                case TextureFiltering::MipmapLinear:
                    return GL_LINEAR_MIPMAP_LINEAR;
                default:
                    return GL_LINEAR;
            }
        };

        glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, filterToGL(settings.MinFilter));
        glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, filterToGL(settings.MagFilter));

        // -----------------------------
        // Generate mipmaps if necessary
        // -----------------------------
        if (settings.MinFilter == TextureFiltering::MipmapNearest ||
            settings.MinFilter == TextureFiltering::MipmapLinear)
        {
            glGenerateTextureMipmap(m_RendererID);
        }
    }

    OpenGLTexture2D::~OpenGLTexture2D()
    {
        glDeleteTextures(1, &m_RendererID);
    }

    void OpenGLTexture2D::SetData(void* data, uint32_t size)
    {
        uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
        TI_CORE_ASSERT(size == m_Width * m_Height * bpp, "Data must be entire texture (expected: {}, got: {} bytes)!",
                       m_Width * m_Height * bpp, size);
        glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
    }

    void OpenGLTexture2D::Bind(uint32_t slot) const
    {
        glBindTextureUnit(slot, m_RendererID);
    }

    void OpenGLTexture2D::Reload(const std::string& path, TextureSettings settings)
    {
        // Create a temporary texture (loads and creates GL texture), then swap internals so
        // existing shared_ptrs keep referring to the same object instance while its content
        // is replaced by the newly loaded texture.
        OpenGLTexture2D tmp(path, settings);

        std::swap(m_Path, tmp.m_Path);
        std::swap(m_Width, tmp.m_Width);
        std::swap(m_Height, tmp.m_Height);
        std::swap(m_RendererID, tmp.m_RendererID);
        std::swap(m_InternalFormat, tmp.m_InternalFormat);
        std::swap(m_DataFormat, tmp.m_DataFormat);
        std::swap(m_BindlessHandle, tmp.m_BindlessHandle);
        std::swap(m_HandleResident, tmp.m_HandleResident);
        std::swap(m_CreatedHandle, tmp.m_CreatedHandle);
    }

    uint64_t OpenGLTexture2D::GetBindlessHandle()
    {
        if (Debug::isRenderdocAttached())
        {
            return 0; // TODO: Fix renderdoc bindless texture issue
        }
        if (m_BindlessHandle == 0)
            m_BindlessHandle = glGetTextureHandleARB(m_RendererID);
        return m_BindlessHandle;
    }

    void OpenGLTexture2D::MakeHandleResident()
    {
        if (Debug::isRenderdocAttached())
        {
            return; // TODO: Fix renderdoc bindless texture issue
        }
        m_CreatedHandle = true;
        if (!m_HandleResident)
        {
            glMakeTextureHandleResidentARB(GetBindlessHandle());
            m_HandleResident = true;
        }
    }

    void OpenGLTexture2D::MakeHandleNonResident()
    {
        if (Debug::isRenderdocAttached())
        {
            return; // TODO: Fix renderdoc bindless texture issue
        }
        m_CreatedHandle = true;
        if (m_HandleResident)
        {
            glMakeTextureHandleNonResidentARB(m_BindlessHandle);
            m_HandleResident = false;
        }
    }
} // namespace Titan