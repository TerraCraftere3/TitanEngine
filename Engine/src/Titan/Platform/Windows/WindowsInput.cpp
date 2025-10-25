#include "Titan/Platform/Windows/WindowsInput.h"
#include "Titan/Core/Application.h"
#include "Titan/PCH.h"

namespace Titan
{
    Scope<Input> Input::s_Instance = CreateScope<WindowsInput>();

    bool WindowsInput::IsKeyPressedImpl(KeyCode keycode)
    {
        GLFWwindow* window = static_cast<GLFWwindow*>(Application::GetInstance()->GetWindow().GetNativeWindow());
        auto state = glfwGetKey(window, static_cast<int32_t>(keycode));
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool WindowsInput::IsMouseButtonPressedImpl(MouseButton button)
    {
        GLFWwindow* window = static_cast<GLFWwindow*>(Application::GetInstance()->GetWindow().GetNativeWindow());
        auto state = glfwGetMouseButton(window, static_cast<int32_t>(button));
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