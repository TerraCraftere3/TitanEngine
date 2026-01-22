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

        // Set polygon mode
        GLenum polygonMode = GL_FILL;
        switch (m_PolygonMode)
        {
            case PolygonMode::Fill:
                polygonMode = GL_FILL;
                break;
            case PolygonMode::Line:
                polygonMode = GL_LINE;
                break;
            case PolygonMode::Point:
                polygonMode = GL_POINT;
                break;
        }
        glPolygonMode(GL_FRONT_AND_BACK, polygonMode);

        // Set cull mode
        if (m_CullMode == CullMode::None)
        {
            glDisable(GL_CULL_FACE);
        }
        else
        {
            glEnable(GL_CULL_FACE);
            GLenum cullMode = GL_BACK;
            switch (m_CullMode)
            {
                case CullMode::Front:
                    cullMode = GL_FRONT;
                    break;
                case CullMode::Back:
                    cullMode = GL_BACK;
                    break;
                case CullMode::FrontAndBack:
                    cullMode = GL_FRONT_AND_BACK;
                    break;
                case CullMode::None:
                    break; // Already handled
            }
            glCullFace(cullMode);
        }

        // Set front face winding
        GLenum frontFace = m_FrontFace == FrontFace::Clockwise ? GL_CW : GL_CCW;
        glFrontFace(frontFace);

        // Set depth bias
        if (m_DepthBiasConstant != 0.0f || m_DepthBiasSlope != 0.0f)
        {
            glEnable(GL_POLYGON_OFFSET_FILL);
            glEnable(GL_POLYGON_OFFSET_LINE);
            glEnable(GL_POLYGON_OFFSET_POINT);
            glPolygonOffset(m_DepthBiasSlope, m_DepthBiasConstant);
        }
        else
        {
            glDisable(GL_POLYGON_OFFSET_FILL);
            glDisable(GL_POLYGON_OFFSET_LINE);
            glDisable(GL_POLYGON_OFFSET_POINT);
        }

        // Set line width
        glLineWidth(m_LineWidth);

        // Set depth testing
        if (m_DepthTestEnabled)
        {
            glEnable(GL_DEPTH_TEST);
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }

        // Set depth function
        GLenum depthFunc = GL_LESS;
        switch (m_DepthFunction)
        {
            case DepthFunc::Never:
                depthFunc = GL_NEVER;
                break;
            case DepthFunc::Less:
                depthFunc = GL_LESS;
                break;
            case DepthFunc::Equal:
                depthFunc = GL_EQUAL;
                break;
            case DepthFunc::LessEqual:
                depthFunc = GL_LEQUAL;
                break;
            case DepthFunc::Greater:
                depthFunc = GL_GREATER;
                break;
            case DepthFunc::NotEqual:
                depthFunc = GL_NOTEQUAL;
                break;
            case DepthFunc::GreaterEqual:
                depthFunc = GL_GEQUAL;
                break;
            case DepthFunc::Always:
                depthFunc = GL_ALWAYS;
                break;
        }
        glDepthFunc(depthFunc);

        // Set depth write
        glDepthMask(m_DepthWriteEnabled ? GL_TRUE : GL_FALSE);
    }

    void OpenGLPipelineState::SetShader(const Ref<Shader>& shader)
    {
        m_Shader = shader;
    }

    void OpenGLPipelineState::SetVertexArray(const Ref<VertexArray>& vertexArray)
    {
        m_VertexArray = vertexArray;
    }

    void OpenGLPipelineState::SetUniformBuffer(const Ref<UniformBuffer>& uniformBuffer, uint32_t bindingPoint)
    {
        m_UniformBuffers.push_back({uniformBuffer, bindingPoint});
    }

    void OpenGLPipelineState::SetShaderStorageBuffer(const Ref<ShaderStorageBuffer>& storageBuffer,
                                                     uint32_t bindingPoint)
    {
        m_StorageBuffers.push_back({storageBuffer, bindingPoint});
    }

    void OpenGLPipelineState::SetTexture(const Ref<Texture2D>& texture, uint32_t slot)
    {
        m_Textures.push_back({texture, slot});
    }

    void OpenGLPipelineState::SetCubemap(const Ref<Cubemap>& cubemap, uint32_t slot)
    {
        m_Cubemaps.push_back({cubemap, slot});
    }

    void OpenGLPipelineState::SetDepthFunction(DepthFunc function)
    {
        m_DepthFunction = function;
    }

    void OpenGLPipelineState::SetPolygonMode(PolygonMode mode)
    {
        m_PolygonMode = mode;
    }

    void OpenGLPipelineState::SetCullMode(CullMode mode)
    {
        m_CullMode = mode;
    }

    void OpenGLPipelineState::SetFrontFace(FrontFace face)
    {
        m_FrontFace = face;
    }

    void OpenGLPipelineState::SetDepthBias(float constantFactor, float slopeFactor)
    {
        m_DepthBiasConstant = constantFactor;
        m_DepthBiasSlope = slopeFactor;
    }

    void OpenGLPipelineState::SetLineWidth(float width)
    {
        m_LineWidth = width;
    }

    void OpenGLPipelineState::SetDepthTestEnabled(bool enabled)
    {
        m_DepthTestEnabled = enabled;
    }

    void OpenGLPipelineState::SetDepthWriteEnabled(bool enabled)
    {
        m_DepthWriteEnabled = enabled;
    }

    void OpenGLPipelineState::ClearBindings()
    {
        m_UniformBuffers.clear();
        m_StorageBuffers.clear();
        m_Textures.clear();
        m_Cubemaps.clear();
    }
} // namespace Titan
