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
        OpenGLTexture2D(const std::string& path, TextureSettings settings);
        OpenGLTexture2D(uint32_t width, uint32_t height);
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

        virtual uint64_t GetBindlessHandle() override;
        virtual void MakeHandleResident() override;
        virtual void MakeHandleNonResident() override;

        virtual bool operator==(const Texture& other) const override
        {
            return m_RendererID == ((OpenGLTexture2D&)other).m_RendererID;
        }

    private:
        std::string m_Path;
        uint32_t m_Width, m_Height;
        uint32_t m_RendererID;
        GLenum m_InternalFormat, m_DataFormat;

        uint64_t m_BindlessHandle = 0;
        bool m_HandleResident = false;
    };

} // namespace Titan