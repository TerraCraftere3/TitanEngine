#pragma once

#include "Titan/Renderer/Texture.h"
// clang-format off
#ifdef APIENTRY
    #undef APIENTRY
#endif
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on
namespace Titan
{
    class OpenGLTexture2D : public Texture2D
    {
    public:
        OpenGLTexture2D(void* ptr, const std::string& path);
        OpenGLTexture2D(const std::string& path, TextureSettings settings);
        OpenGLTexture2D(uint32_t width, uint32_t height, TextureFormat format = TextureFormat::RGBA8,
                        TextureSettings settings = TextureSettings());
        virtual ~OpenGLTexture2D();

        virtual uint32_t GetWidth() const override { return m_Width; }
        virtual uint32_t GetHeight() const override { return m_Height; }

        virtual std::string GetPath() const override { return m_Path; }

        virtual void SetData(void* data, uint32_t size) override;

        inline void* GetNativeTexture() const override
        {
            return reinterpret_cast<void*>(static_cast<uintptr_t>(m_RendererID));
        }

        virtual void Bind(uint32_t slot = 0) const override;

        virtual void Reload(const std::string& path, TextureSettings settings) override;

        virtual uint64_t GetBindlessHandle() override;
        virtual void MakeHandleResident() override;
        virtual void MakeHandleNonResident() override;
        virtual bool isValidBindlessHandle() override { return m_CreatedHandle; };

        virtual bool operator==(const Texture2D& other) const override
        {
            return m_RendererID == ((OpenGLTexture2D&)other).m_RendererID;
        }

        void ReplaceTextureFromPixels(unsigned char* data, int width, int height, TextureSettings settings,
                                      int channels, bool isSVG);

        static Ref<Texture2D> CreateAsync(const std::string& path, TextureSettings settings);

        uint32_t GetRendererID() const { return m_RendererID; }

    private:
        std::string m_Path;
        uint32_t m_Width, m_Height;
        uint32_t m_RendererID;
        GLenum m_InternalFormat, m_DataFormat;

        uint64_t m_BindlessHandle = 0;
        bool m_HandleResident = false;
        bool m_CreatedHandle = false;
    };

} // namespace Titan