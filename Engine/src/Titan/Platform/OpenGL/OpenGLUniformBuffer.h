#pragma once

#include "Titan/Renderer/RHI/UniformBuffer.h"

namespace Titan
{

    class OpenGLUniformBuffer : public UniformBuffer
    {
    public:
        OpenGLUniformBuffer(uint32_t size);
        virtual ~OpenGLUniformBuffer();

        virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) override;

        uint32_t GetRendererID() const { return m_RendererID; }

    private:
        uint32_t m_RendererID = 0;
    };
} // namespace Titan