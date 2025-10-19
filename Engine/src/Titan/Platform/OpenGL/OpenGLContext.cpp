#include "Titan/Platform/OpenGL/OpenGLContext.h"

namespace Titan
{

    OpenGLContext::OpenGLContext(GLFWwindow* window)
    {
        TI_CORE_ASSERT(window, "Window doesnt exist! Graphics context cant be created!");
        m_Window = window;
    }

    void OpenGLContext::Init()
    {
        glfwMakeContextCurrent(m_Window);
        int success = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        TI_CORE_ASSERT(success, "Couldnt load Glad (OpenGL)!")
    }

    void OpenGLContext::Swapbuffers()
    {
        glfwSwapBuffers(m_Window);
    }

} // namespace Titan