#pragma once

#include <glad/glad.h>
#include <algorithm>
#include <cstdint>
#include <string>
#include "Titan/Renderer/Buffer.h"

namespace Titan
{
    // Generic OpenGL buffer abstraction parameterized by target binding point.
    template <GLenum Target>
    class OpenGLBufferBase
    {
    public:
        // Allocate with given size (no initial data)
        OpenGLBufferBase(uint32_t size, GLenum usage) : m_Size(size), m_Usage(usage)
        {
            glCreateBuffers(1, &m_RendererID);
            glBindBuffer(Target, m_RendererID);
            glBufferData(Target, (GLsizeiptr)size, nullptr, usage);
        }

        // Allocate and upload initial data
        OpenGLBufferBase(const void* data, uint32_t size, GLenum usage) : m_Size(size), m_Usage(usage)
        {
            glCreateBuffers(1, &m_RendererID);
            glBindBuffer(Target, m_RendererID);
            glBufferData(Target, (GLsizeiptr)size, data, usage);
        }

        // Move constructor
        OpenGLBufferBase(OpenGLBufferBase&& other) noexcept
            : m_RendererID(other.m_RendererID), m_Size(other.m_Size), m_Usage(other.m_Usage)
        {
            other.m_RendererID = 0;
            other.m_Size = 0;
            other.m_Usage = 0;
        }

        // Move assignment
        OpenGLBufferBase& operator=(OpenGLBufferBase&& other) noexcept
        {
            if (this != &other)
            {
                if (m_RendererID)
                    glDeleteBuffers(1, &m_RendererID);

                m_RendererID = other.m_RendererID;
                m_Size = other.m_Size;
                m_Usage = other.m_Usage;

                other.m_RendererID = 0;
                other.m_Size = 0;
                other.m_Usage = 0;
            }
            return *this;
        }

        // non-copyable (OpenGL resource)
        OpenGLBufferBase(const OpenGLBufferBase&) = delete;
        OpenGLBufferBase& operator=(const OpenGLBufferBase&) = delete;

        virtual ~OpenGLBufferBase()
        {
            if (m_RendererID)
                glDeleteBuffers(1, &m_RendererID);
        }

        void BindRaw() const { glBindBuffer(Target, m_RendererID); }
        void UnbindRaw() const { glBindBuffer(Target, 0); }

        void SetSubData(const void* data, uint32_t size, uint32_t offset = 0)
        {
            // auto-resize if necessary
            if (offset + size > m_Size)
            {
                Resize(std::max<uint32_t>(offset + size, m_Size * 2u), m_Usage);
            }
            glBindBuffer(Target, m_RendererID);
            glBufferSubData(Target, (GLintptr)offset, (GLsizeiptr)size, data);
        }

        // Simple full-map
        void* Map(GLbitfield access = GL_READ_WRITE)
        {
            glBindBuffer(Target, m_RendererID);
            return glMapBuffer(Target, access);
        }

        // Map a range (recommended over full map for performance)
        void* MapRange(uint32_t offset, uint32_t length, GLbitfield access)
        {
            glBindBuffer(Target, m_RendererID);
            return glMapBufferRange(Target, (GLintptr)offset, (GLsizeiptr)length, access);
        }

        void Unmap()
        {
            glBindBuffer(Target, m_RendererID);
            glUnmapBuffer(Target);
        }

        // Resize preserving old contents (copies min(old,new) bytes)
        void Resize(uint32_t newSize, GLenum usage)
        {
            if (newSize == m_Size)
            {
                m_Usage = usage;
                return;
            }

            GLuint newBuffer = 0;
            glCreateBuffers(1, &newBuffer);
            glBindBuffer(Target, newBuffer);
            glBufferData(Target, (GLsizeiptr)newSize, nullptr, usage);

            if (m_RendererID && m_Size > 0)
            {
                // copy min bytes
                uint32_t copySize = std::min(m_Size, newSize);
                glBindBuffer(GL_COPY_READ_BUFFER, m_RendererID);
                glBindBuffer(GL_COPY_WRITE_BUFFER, newBuffer);
                glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, (GLsizeiptr)copySize);
            }

            if (m_RendererID)
                glDeleteBuffers(1, &m_RendererID);

            m_RendererID = newBuffer;
            m_Size = newSize;
            m_Usage = usage;

            // unbind copy targets
            glBindBuffer(GL_COPY_READ_BUFFER, 0);
            glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
        }

        // Copy contents from another buffer into this (resizes if needed)
        void CopyFrom(const OpenGLBufferBase& src, uint32_t size, uint32_t srcOffset = 0, uint32_t dstOffset = 0)
        {
            uint32_t required = dstOffset + size;
            if (required > m_Size)
                Resize(required, m_Usage);

            glBindBuffer(GL_COPY_READ_BUFFER, src.m_RendererID);
            glBindBuffer(GL_COPY_WRITE_BUFFER, m_RendererID);
            glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, (GLintptr)srcOffset, (GLintptr)dstOffset,
                                (GLsizeiptr)size);

            glBindBuffer(GL_COPY_READ_BUFFER, 0);
            glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
        }

        // Bind to indexed binding point (for UBO/SSBO/Atomic counters)
        void BindBase(GLuint index) const { glBindBufferBase(Target, index, m_RendererID); }

        void BindRange(GLuint index, GLintptr offset, GLsizeiptr size) const
        {
            glBindBufferRange(Target, index, m_RendererID, offset, size);
        }

        // Type-safe map helper
        template <typename T>
        T* MapTyped(GLbitfield access = GL_READ_WRITE)
        {
            return reinterpret_cast<T*>(Map(access));
        }

        uint32_t GetRendererID() const { return m_RendererID; }
        uint32_t GetSize() const { return m_Size; }
        GLenum GetUsage() const { return m_Usage; }

        // Debug label (requires GL_KHR_debug or OpenGL 4.3+)
        void SetDebugName(const std::string& name)
        {
#if defined(GL_KHR_debug) || GL_VERSION_4_3
            if (m_RendererID)
                glObjectLabel(GL_BUFFER, m_RendererID, static_cast<GLsizei>(name.size()), name.c_str());
#endif
        }

    protected:
        uint32_t m_RendererID = 0;
        uint32_t m_Size = 0;
        GLenum m_Usage = GL_STATIC_DRAW;
    };

    class OpenGLVertexBuffer : public VertexBuffer, protected OpenGLBufferBase<GL_ARRAY_BUFFER>
    {
    public:
        OpenGLVertexBuffer(uint32_t size) : OpenGLBufferBase<GL_ARRAY_BUFFER>(size, GL_DYNAMIC_DRAW) {}

        OpenGLVertexBuffer(float* vertices, uint32_t sizeInBytes)
            : OpenGLBufferBase<GL_ARRAY_BUFFER>(vertices, sizeInBytes, GL_STATIC_DRAW)
        {
        }

        OpenGLVertexBuffer(const void* data, uint32_t sizeInBytes)
            : OpenGLBufferBase<GL_ARRAY_BUFFER>(data, sizeInBytes, GL_STATIC_DRAW)
        {
        }

        virtual ~OpenGLVertexBuffer() override = default;

        virtual void Bind() const override { BindRaw(); }
        virtual void Unbind() const override { UnbindRaw(); }

        virtual void SetData(const void* data, uint32_t size) override { SetSubData(data, size, 0); }

        virtual const BufferLayout& GetLayout() const override { return m_Layout; }
        virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; }

        uint32_t GetRendererID() const { return m_RendererID; }

    private:
        BufferLayout m_Layout;
    };

    class OpenGLIndexBuffer : public IndexBuffer, protected OpenGLBufferBase<GL_ELEMENT_ARRAY_BUFFER>
    {
    public:
        OpenGLIndexBuffer(uint32_t* indices, uint32_t count)
            : OpenGLBufferBase<GL_ELEMENT_ARRAY_BUFFER>(indices, count * sizeof(uint32_t), GL_STATIC_DRAW),
              m_Count(count)
        {
        }

        OpenGLIndexBuffer(const uint32_t* indices, uint32_t count)
            : OpenGLBufferBase<GL_ELEMENT_ARRAY_BUFFER>(indices, count * sizeof(uint32_t), GL_STATIC_DRAW),
              m_Count(count)
        {
        }

        OpenGLIndexBuffer(uint32_t count, GLenum usage = GL_STATIC_DRAW)
            : OpenGLBufferBase<GL_ELEMENT_ARRAY_BUFFER>(count * sizeof(uint32_t), usage), m_Count(count)
        {
        }

        virtual ~OpenGLIndexBuffer() override = default;

        virtual void Bind() const override { BindRaw(); }
        virtual void Unbind() const override { UnbindRaw(); }

        virtual uint32_t GetCount() const override { return m_Count; }

        uint32_t GetRendererID() const { return m_RendererID; }

    private:
        uint32_t m_Count = 0;
    };

    class OpenGLShaderStorageBuffer : public ShaderStorageBuffer, protected OpenGLBufferBase<GL_SHADER_STORAGE_BUFFER>
    {
    public:
        OpenGLShaderStorageBuffer(uint32_t size) : OpenGLBufferBase<GL_SHADER_STORAGE_BUFFER>(size, GL_DYNAMIC_COPY) {}

        virtual ~OpenGLShaderStorageBuffer() override = default;

        void SetData(const void* data, uint32_t size, uint32_t offset = 0) override { SetSubData(data, size, offset); }

        void Bind(uint32_t slot) const override
        {
            BindRaw();
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot, m_RendererID);
        }

        void Unbind(uint32_t slot) const override
        {
            UnbindRaw();
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, slot, 0);
        }

        uint32_t GetRendererID() const { return m_RendererID; }
    };

} // namespace Titan
