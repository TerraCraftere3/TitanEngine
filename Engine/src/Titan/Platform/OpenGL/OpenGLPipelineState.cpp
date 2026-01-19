#include "OpenGLPipelineState.h"
#include "Titan/Core.h"
#include "Titan/Platform/OpenGL/OpenGLBuffer.h"
#include "Titan/Platform/OpenGL/OpenGLCubemap.h"
#include "Titan/Platform/OpenGL/OpenGLShader.h"
#include "Titan/Platform/OpenGL/OpenGLTexture.h"
#include "Titan/Platform/OpenGL/OpenGLUniformBuffer.h"
#include "Titan/Platform/OpenGL/OpenGLVertexArray.h"

#include "Titan/Renderer/Buffer.h"
#include "Titan/Renderer/Cubemap.h"
#include "Titan/Renderer/Texture.h"
#include "Titan/Renderer/UniformBuffer.h"
#include "Titan/Renderer/VertexArray.h"

#include <algorithm>

namespace Titan
{
    static constexpr uint32_t s_MaxBindingSlots = 32;

    static uint32_t g_CurrentShader = 0;
    static uint32_t g_CurrentVAO = 0;
    static uint32_t g_CurrentTextures[s_MaxBindingSlots] = {0};
    static uint32_t g_CurrentCubemaps[s_MaxBindingSlots] = {0};
    static uint32_t g_CurrentUBOs[s_MaxBindingSlots] = {0};
    static uint32_t g_CurrentSSBOs[s_MaxBindingSlots] = {0};
    static int32_t g_CurrentActiveTextureUnit = -1;

    void OpenGLPipelineState::ResetCachedState()
    {
        g_CurrentShader = 0;
        g_CurrentVAO = 0;
        std::fill(std::begin(g_CurrentTextures), std::end(g_CurrentTextures), 0);
        std::fill(std::begin(g_CurrentCubemaps), std::end(g_CurrentCubemaps), 0);
        std::fill(std::begin(g_CurrentUBOs), std::end(g_CurrentUBOs), 0);
        std::fill(std::begin(g_CurrentSSBOs), std::end(g_CurrentSSBOs), 0);
        g_CurrentActiveTextureUnit = -1;
    }

    OpenGLPipelineState::OpenGLPipelineState() : m_Shader(nullptr), m_VertexArray(nullptr) {}

    void OpenGLPipelineState::Bind() const
    {
        // Bind shader first
        if (m_Shader)
        {
            const auto shader = std::static_pointer_cast<OpenGLShader>(m_Shader);
            const uint32_t rendererID = shader->GetRendererID();
            if (g_CurrentShader != rendererID)
            {
                glUseProgram(rendererID);
                g_CurrentShader = rendererID;
            }
        }

        // Bind vertex array
        if (m_VertexArray)
        {
            const auto vao = std::static_pointer_cast<OpenGLVertexArray>(m_VertexArray);
            const uint32_t rendererID = vao->GetRendererID();
            if (g_CurrentVAO != rendererID)
            {
                glBindVertexArray(rendererID);
                g_CurrentVAO = rendererID;
            }
        }

        // Bind all uniform buffers
        for (size_t i = 0; i < m_UniformBuffers.size(); ++i)
        {
            const uint32_t bindingPoint = m_UniformBuffers[i].second;
            TI_CORE_ASSERT(bindingPoint < s_MaxBindingSlots, "UBO binding point out of range");

            const auto ubo = std::static_pointer_cast<OpenGLUniformBuffer>(m_UniformBuffers[i].first);
            const uint32_t rendererID = ubo->GetRendererID();
            if (g_CurrentUBOs[bindingPoint] != rendererID)
            {
                glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, rendererID);
                g_CurrentUBOs[bindingPoint] = rendererID;
            }
        }

        // Bind all storage buffers
        for (size_t i = 0; i < m_StorageBuffers.size(); ++i)
        {
            const uint32_t bindingPoint = m_StorageBuffers[i].second;
            TI_CORE_ASSERT(bindingPoint < s_MaxBindingSlots, "SSBO binding point out of range");

            const auto ssbo = std::static_pointer_cast<OpenGLShaderStorageBuffer>(m_StorageBuffers[i].first);
            const uint32_t rendererID = ssbo->GetRendererID();
            if (g_CurrentSSBOs[bindingPoint] != rendererID)
            {
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, rendererID);
                g_CurrentSSBOs[bindingPoint] = rendererID;
            }
        }

        // Bind all textures
        for (size_t i = 0; i < m_Textures.size(); ++i)
        {
            const uint32_t slot = m_Textures[i].second;
            TI_CORE_ASSERT(slot < s_MaxBindingSlots, "Texture slot out of range");

            const auto texture = std::static_pointer_cast<OpenGLTexture2D>(m_Textures[i].first);
            const uint32_t rendererID = texture->GetRendererID();
            if (g_CurrentTextures[slot] != rendererID)
            {
                if (g_CurrentActiveTextureUnit != static_cast<int32_t>(slot))
                {
                    glActiveTexture(GL_TEXTURE0 + slot);
                    g_CurrentActiveTextureUnit = static_cast<int32_t>(slot);
                }
                glBindTexture(GL_TEXTURE_2D, rendererID);
                g_CurrentTextures[slot] = rendererID;
            }
        }

        // Bind all cubemaps
        for (size_t i = 0; i < m_Cubemaps.size(); ++i)
        {
            const uint32_t slot = m_Cubemaps[i].second;
            TI_CORE_ASSERT(slot < s_MaxBindingSlots, "Cubemap slot out of range");

            const auto cubemap = std::static_pointer_cast<OpenGLCubemap>(m_Cubemaps[i].first);
            const uint32_t rendererID = cubemap->GetRendererID();
            if (g_CurrentCubemaps[slot] != rendererID)
            {
                if (g_CurrentActiveTextureUnit != static_cast<int32_t>(slot))
                {
                    glActiveTexture(GL_TEXTURE0 + slot);
                    g_CurrentActiveTextureUnit = static_cast<int32_t>(slot);
                }
                glBindTexture(GL_TEXTURE_CUBE_MAP, rendererID);
                g_CurrentCubemaps[slot] = rendererID;
            }
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
