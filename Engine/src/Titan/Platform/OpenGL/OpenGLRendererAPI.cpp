#include "OpenGLRendererAPI.h"
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

    void OpenGLRendererAPI::DrawArrays(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
    {
        vertexArray->Bind();
        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }

    void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
    {
        vertexArray->Bind();
        uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
    }

    void OpenGLRendererAPI::DrawIndexedInstanced(const Ref<VertexArray>& vertexArray, uint32_t indexCount,
                                                 uint32_t instanceCount)
    {
        vertexArray->Bind();
        uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
        glDrawElementsInstanced(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr, instanceCount);
    }

    void OpenGLRendererAPI::DrawIndexedInstancedBaseIndex(const Ref<VertexArray>& vertexArray, uint32_t indexCount,
                                                          uint32_t instanceCount, uint32_t baseIndex)
    {
        vertexArray->Bind();
        glDrawElementsInstancedBaseVertex(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT,
                                          (void*)(baseIndex * sizeof(uint32_t)), instanceCount, 0);
    }

    void OpenGLRendererAPI::DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
    {
        vertexArray->Bind();
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

} // namespace Titan