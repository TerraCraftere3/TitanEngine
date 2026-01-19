#pragma once

#include "Titan/Renderer/Cubemap.h"
// clang-format off
#ifdef APIENTRY
    #undef APIENTRY
#endif
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

namespace Titan
{

    // Convert from equirectangular to a cubemap
    class OpenGLCubemap : public Cubemap
    {
    public:
        OpenGLCubemap();
        OpenGLCubemap(const std::string&
                          path /*e.g. example.hdr (Equirectangular) */); // if loaded from 6 textures or HDR → cubemap
        virtual ~OpenGLCubemap();

        // --- Cubemap API ---
        virtual uint32_t GetWidth() const override { return m_Width; }
        virtual uint32_t GetHeight() const override { return m_Height; }

        virtual std::string GetPath() const override { return m_Path; }

        virtual void SetData(void* data, uint32_t size) override;
        virtual void SetFaceData(uint32_t face, void* data, uint32_t size) override;

        virtual void Bind(uint32_t slot = 0) const override;
        virtual void* GetNativeTexture() const override { return (void*)(uintptr_t)m_RendererID; }

        virtual Ref<Cubemap> CreateIrradianceMap(uint32_t resolution = 32) override;

        uint32_t GetRendererID() const { return m_RendererID; }

    private:
        uint32_t m_RendererID = 0; // GLuint texture ID

        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
        uint32_t m_MipLevels = 1;

        std::string m_Path = "";
    };

} // namespace Titan