#include "OpenGLShaderStorageBuffer.h"
#include "Titan/PCH.h"

namespace Titan
{
    OpenGLShaderStorageBuffer::OpenGLShaderStorageBuffer(uint32_t size, uint32_t binding) : m_Binding(binding)
    {
        glGenBuffers(1, &m_RendererID);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID);
        glBufferData(GL_SHADER_STORAGE_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, m_RendererID);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    OpenGLShaderStorageBuffer::~OpenGLShaderStorageBuffer()
    {
        glDeleteBuffers(1, &m_RendererID);
    }

    void OpenGLShaderStorageBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    void OpenGLShaderStorageBuffer::Bind() const
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_Binding, m_RendererID);
    }

    void OpenGLShaderStorageBuffer::Unbind() const
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_Binding, 0);
    }
} // namespace Titan
