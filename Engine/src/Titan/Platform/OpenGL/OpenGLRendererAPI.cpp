#include "OpenGLRendererAPI.h"
#include "OpenGLFramebuffer.h"
#include "OpenGLPipelineState.h"
#include "Titan/PCH.h"

// clang-format off
#ifdef APIENTRY
    #undef APIENTRY
#endif
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on
namespace Titan
{
    void OpenGLRendererAPI::Init()
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        glEnable(GL_LINE_SMOOTH);
    }

    void OpenGLRendererAPI::BeginRenderPass(Ref<Framebuffer> framebuffer, std::string debugName)
    {
#ifdef TI_BUILD_DEBUG
        glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, debugName.c_str());
#endif

        OpenGLPipelineState::ResetCachedState();

        glBindFramebuffer(GL_FRAMEBUFFER, std::dynamic_pointer_cast<OpenGLFramebuffer>(framebuffer)->GetRendererID());
        glViewport(0, 0, framebuffer->GetWidth(), framebuffer->GetHeight());
    }

    void OpenGLRendererAPI::EndRenderPass()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

#ifdef TI_BUILD_DEBUG
        glPopDebugGroup();
#endif
    }

    void OpenGLRendererAPI::SetPolygonMode(PolygonMode mode)
    {
        GLenum glMode = GL_FILL;

        switch (mode)
        {
            case PolygonMode::Fill:
                glMode = GL_FILL;
                break;
            case PolygonMode::Line:
                glMode = GL_LINE;
                break;
            case PolygonMode::Point:
                glMode = GL_POINT;
                break;
        }

        glPolygonMode(GL_FRONT_AND_BACK, glMode);
    }

    void OpenGLRendererAPI::SetDepthFunc(DepthFunc function)
    {
        GLenum glFunc = GL_LESS;

        switch (function)
        {
            case DepthFunc::Never:
                glFunc = GL_NEVER;
                break;
            case DepthFunc::Less:
                glFunc = GL_LESS;
                break;
            case DepthFunc::Equal:
                glFunc = GL_EQUAL;
                break;
            case DepthFunc::LessEqual:
                glFunc = GL_LEQUAL;
                break;
            case DepthFunc::Greater:
                glFunc = GL_GREATER;
                break;
            case DepthFunc::NotEqual:
                glFunc = GL_NOTEQUAL;
                break;
            case DepthFunc::GreaterEqual:
                glFunc = GL_GEQUAL;
                break;
            case DepthFunc::Always:
                glFunc = GL_ALWAYS;
                break;
        }

        glDepthFunc(glFunc);
    }

    void OpenGLRendererAPI::SetClearColor(const glm::vec4& color)
    {
        glClearColor(color.r, color.g, color.b, color.a);
    }

    void OpenGLRendererAPI::Clear()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void OpenGLRendererAPI::DrawArrays(uint32_t vertexCount)
    {
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }

    void OpenGLRendererAPI::DrawIndexed(uint32_t indexCount)
    {
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    }

    void OpenGLRendererAPI::DrawIndexedInstanced(uint32_t indexCount, uint32_t instanceCount)
    {
        glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr, instanceCount);
    }

    void OpenGLRendererAPI::DrawIndexedInstancedBaseIndex(uint32_t indexCount, uint32_t instanceCount,
                                                          uint32_t baseIndex)
    {
        glDrawElementsInstancedBaseVertex(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT,
                                          (void*)(baseIndex * sizeof(uint32_t)), instanceCount, 0);
    }

    void OpenGLRendererAPI::DrawLines(uint32_t vertexCount)
    {
        glDrawArrays(GL_LINES, 0, vertexCount);
    }

    void OpenGLRendererAPI::SetLineWidth(float width)
    {
        glLineWidth(width);
    }

    void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        glViewport(x, y, width, height);
    }

    const RendererAPI::Backend& OpenGLRendererAPI::GetBackend() const
    {
        static Backend backend;

        static bool initialized = false;
        if (!initialized)
        {
            backend.Vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
            backend.Renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
            backend.Version = reinterpret_cast<const char*>(glGetString(GL_VERSION));

            GLint maxTextureUnits = 0;
            glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
            backend.MaxTextureUnits = static_cast<uint32_t>(maxTextureUnits);

            GLint maxTextureSize = 0;
            glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
            backend.MaxTextureSize = static_cast<uint32_t>(maxTextureSize);

            backend.SupportsComputeShaders = GLAD_GL_ARB_compute_shader;
            backend.SupportsTessellationShaders = GLAD_GL_ARB_tessellation_shader;
            backend.SupportsGeometryShaders = GLAD_GL_ARB_geometry_shader4;

            backend.SupportsRaytracing = false;

            initialized = true;
        }

        return backend;
    }

} // namespace Titan