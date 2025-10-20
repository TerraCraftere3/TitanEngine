#include "Titan/Platform/OpenGL/OpenGLContext.h"

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
    }

    void OpenGLContext::Swapbuffers()
    {
        glfwSwapBuffers(m_Window);
    }

} // namespace Titan