#pragma once

#include "Titan/Renderer/UniformBuffer.h"

namespace Titan
{

    class OpenGLUniformBuffer : public UniformBuffer
    {
    public:
        OpenGLUniformBuffer(uint32_t size, uint32_t binding);
        virtual ~OpenGLUniformBuffer();

        virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;
        virtual void Bind() override;

    private:
        uint32_t m_RendererID = 0;
        uint32_t m_BindingPoint = 0;
    };
} // namespace Titan