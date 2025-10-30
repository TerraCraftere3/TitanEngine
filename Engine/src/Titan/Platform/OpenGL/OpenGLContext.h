#pragma once

#include "Titan/Core.h"
#include "Titan/PCH.h"
#include "Titan/Renderer/GraphicsContext.h"
// clang-format off
#ifdef APIENTRY
    #undef APIENTRY
#endif
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on
namespace Titan
{

    class OpenGLContext : public GraphicsContext
    {
    public:
        OpenGLContext(GLFWwindow* window);

        virtual void Init() override;
        virtual void Swapbuffers() override;

    private:
        GLFWwindow* m_Window;
    };

} // namespace Titan