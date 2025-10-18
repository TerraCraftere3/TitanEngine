#include "WindowsInput.h"
#include "Titan/Application.h"
#include "Titan/KeyCodes.h"
#include "Titan/MouseButtonCodes.h"
#include "Titan/PCH.h"

namespace Titan
{
    Input* Input::s_Instance = new WindowsInput();

    bool WindowsInput::IsKeyPressedImpl(int keycode)
    {
        GLFWwindow* window = static_cast<GLFWwindow*>(Application::GetInstance()->GetWindow().GetNativeWindow());
        auto state = glfwGetKey(window, TI_KEY_CODE_TO_GLFW_KEY_CODE(keycode));
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool WindowsInput::IsMouseButtonPressedImpl(int button)
    {
        GLFWwindow* window = static_cast<GLFWwindow*>(Application::GetInstance()->GetWindow().GetNativeWindow());
        auto state = glfwGetMouseButton(window, TI_MOUSE_BTN_TO_GLFW_MOUSE_BTN(button));
        return state == GLFW_PRESS;
    }

    float WindowsInput::GetMouseXImpl()
    {
        GLFWwindow* window = static_cast<GLFWwindow*>(Application::GetInstance()->GetWindow().GetNativeWindow());
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        return (float)xpos;
    }

    float WindowsInput::GetMouseYImpl()
    {
        GLFWwindow* window = static_cast<GLFWwindow*>(Application::GetInstance()->GetWindow().GetNativeWindow());
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        return (float)ypos;
    }

} // namespace Titan