#pragma once

#include "Titan/Renderer/ShaderStorageBuffer.h"
// clang-format off
#ifdef APIENTRY
    #undef APIENTRY
#endif
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

namespace Titan
{
    class OpenGLShaderStorageBuffer : public ShaderStorageBuffer
    {
    public:
        OpenGLShaderStorageBuffer(uint32_t size, uint32_t binding);
        virtual ~OpenGLShaderStorageBuffer() override;

        void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;
        void Bind() const override;
        void Unbind() const override;

    private:
        uint32_t m_RendererID = 0;
        uint32_t m_Binding = 0;
    };
}
