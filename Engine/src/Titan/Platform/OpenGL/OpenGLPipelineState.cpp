#include "OpenGLPipelineState.h"
#include "Titan/Platform/OpenGL/OpenGLShader.h"
#include "Titan/Renderer/Buffer.h"
#include "Titan/Renderer/Cubemap.h"
#include "Titan/Renderer/Texture.h"
#include "Titan/Renderer/UniformBuffer.h"
#include "Titan/Renderer/VertexArray.h"

namespace Titan
{
    OpenGLPipelineState::OpenGLPipelineState() : m_Shader(nullptr), m_VertexArray(nullptr) {}

    void OpenGLPipelineState::Bind() const
    {
        // Bind shader first
        if (m_Shader)
        {
            std::dynamic_pointer_cast<OpenGLShader>(m_Shader)->Bind();
        }

        // Bind vertex array
        if (m_VertexArray)
        {
            m_VertexArray->Bind();
        }

        // Bind all uniform buffers
        for (size_t i = 0; i < m_UniformBuffers.size(); ++i)
        {
            m_UniformBuffers[i].first->Bind();
        }

        // Bind all storage buffers
        for (size_t i = 0; i < m_StorageBuffers.size(); ++i)
        {
            m_StorageBuffers[i].first->Bind();
        }

        // Bind all textures
        for (size_t i = 0; i < m_Textures.size(); ++i)
        {
            m_Textures[i].first->Bind(m_Textures[i].second);
        }

        // Bind all cubemaps
        for (size_t i = 0; i < m_Cubemaps.size(); ++i)
        {
            m_Cubemaps[i].first->Bind(m_Cubemaps[i].second);
        }
    }

    void OpenGLPipelineState::SetShader(const Ref<Shader>& shader)
    {
        m_Shader = shader;
    }

    void OpenGLPipelineState::SetVertexArray(const Ref<VertexArray>& vertexArray)
    {
        m_VertexArray = vertexArray;
    }

    void OpenGLPipelineState::BindUniformBuffer(const Ref<UniformBuffer>& uniformBuffer, uint32_t bindingPoint)
    {
        m_UniformBuffers.push_back({uniformBuffer, bindingPoint});
    }

    void OpenGLPipelineState::BindShaderStorageBuffer(const Ref<ShaderStorageBuffer>& storageBuffer,
                                                      uint32_t bindingPoint)
    {
        m_StorageBuffers.push_back({storageBuffer, bindingPoint});
    }

    void OpenGLPipelineState::BindTexture(const Ref<Texture2D>& texture, uint32_t slot)
    {
        m_Textures.push_back({texture, slot});
    }

    void OpenGLPipelineState::BindCubemap(const Ref<Cubemap>& cubemap, uint32_t slot)
    {
        m_Cubemaps.push_back({cubemap, slot});
    }

    void OpenGLPipelineState::ClearBindings()
    {
        m_UniformBuffers.clear();
        m_StorageBuffers.clear();
        m_Textures.clear();
        m_Cubemaps.clear();
    }
} // namespace Titan
