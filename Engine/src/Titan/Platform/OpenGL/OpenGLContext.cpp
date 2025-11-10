#include "Titan/Platform/OpenGL/OpenGLContext.h"

void APIENTRY OpenGLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                  const GLchar* message, const void* userParam)
{
    std::string sourceStr;
    switch (source)
    {
        case GL_DEBUG_SOURCE_API:
            sourceStr = "API";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            sourceStr = "Window System";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            sourceStr = "Shader Compiler";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:
            sourceStr = "Third Party";
            break;
        case GL_DEBUG_SOURCE_APPLICATION:
            sourceStr = "Application";
            break;
        case GL_DEBUG_SOURCE_OTHER:
            sourceStr = "Other";
            break;
        default:
            sourceStr = "Unknown";
            break;
    }

    std::string typeStr;
    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:
            typeStr = "Error";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            typeStr = "Deprecated Behavior";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            typeStr = "Undefined Behavior";
            break;
        case GL_DEBUG_TYPE_PORTABILITY:
            typeStr = "Portability";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE:
            typeStr = "Performance";
            break;
        case GL_DEBUG_TYPE_MARKER:
            typeStr = "Marker";
            break;
        case GL_DEBUG_TYPE_PUSH_GROUP:
            typeStr = "Push Group";
            break;
        case GL_DEBUG_TYPE_POP_GROUP:
            typeStr = "Pop Group";
            break;
        case GL_DEBUG_TYPE_OTHER:
            typeStr = "Other";
            break;
        default:
            typeStr = "Unknown";
            break;
    }

    std::string severityStr;
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:
            severityStr = "High";
            break;
        case GL_DEBUG_SEVERITY_MEDIUM:
            severityStr = "Medium";
            break;
        case GL_DEBUG_SEVERITY_LOW:
            severityStr = "Low";
            break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:
            severityStr = "Notification";
            break;
        default:
            severityStr = "Unknown";
            break;
    }

    TI_CORE_ERROR("[OpenGL][{}][{}][ID {}][{}] {}", sourceStr, typeStr, id, severityStr, message);
}

namespace Titan
{

    OpenGLContext::OpenGLContext(GLFWwindow* window)
    {
        TI_PROFILE_FUNCTION();
        TI_CORE_ASSERT(window, "Window doesnt exist! Graphics context cant be created!");
        m_Window = window;
    }

    void OpenGLContext::Init()
    {
        TI_PROFILE_FUNCTION();
        glfwMakeContextCurrent(m_Window);
        int success = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        TI_CORE_ASSERT(success, "Couldnt load Glad (OpenGL)!")

        TI_CORE_TRACE("OpenGL Info:");
        TI_CORE_TRACE("\tVendor: {0}", (const char*)glGetString(GL_VENDOR));
        TI_CORE_TRACE("\tRenderer: {0}", (const char*)glGetString(GL_RENDERER));
        TI_CORE_TRACE("\tVersion: {0}", (const char*)glGetString(GL_VERSION));

        GLint flags;
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
        {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(OpenGLDebugCallback, nullptr);
            // Enable all messages
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
        }
        else
        {
            TI_CORE_WARN("OpenGL context does not support debug output!");
        }
    }

    void OpenGLContext::Swapbuffers()
    {
        glfwSwapBuffers(m_Window);
    }

} // namespace Titan