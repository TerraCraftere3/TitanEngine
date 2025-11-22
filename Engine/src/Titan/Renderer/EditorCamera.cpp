#include "EditorCamera.h"
#include "Titan/Core/Input.h"
#include "Titan/Core/KeyCodes.h"
#include "Titan/Core/MouseButtons.h"
#include "Titan/PCH.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Titan
{

    EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip)
        : m_FOV(fov),
          m_AspectRatio(aspectRatio),
          m_NearClip(nearClip),
          m_FarClip(farClip),
          Camera(glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip))
    {
        UpdateView();
    }

    void EditorCamera::UpdateProjection()
    {
        m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
        m_Projection = glm::perspective(glm::radians(m_FOV), m_AspectRatio, m_NearClip, m_FarClip);
    }

    void EditorCamera::UpdateView()
    {
        if (m_FpvEnabled)
        {
            m_Position = m_FpvPosition;
            glm::quat orientation = glm::quat(glm::vec3(glm::radians(-m_FpvPitch), glm::radians(-m_FpvYaw), 0.0f));
            m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation);
            m_ViewMatrix = glm::inverse(m_ViewMatrix);
            return;
        }

        // m_Yaw = m_Pitch = 0.0f; // Lock the camera's rotation
        m_Position = CalculatePosition();

        glm::quat orientation = GetOrientation();
        m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation);
        m_ViewMatrix = glm::inverse(m_ViewMatrix);
    }

    std::pair<float, float> EditorCamera::PanSpeed() const
    {
        float x = min(m_ViewportWidth / 1000.0f, 2.4f); // max = 2.4f
        float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

        float y = min(m_ViewportHeight / 1000.0f, 2.4f); // max = 2.4f
        float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

        return {xFactor, yFactor};
    }

    float EditorCamera::RotationSpeed() const
    {
        return 0.8f;
    }

    float EditorCamera::ZoomSpeed() const
    {
        float distance = m_Distance * 0.2f;
        distance = max(distance, 0.0f);
        float speed = distance * distance;
        speed = min(speed, 100.0f); // max speed = 100
        return speed;
    }

    void EditorCamera::OnUpdate(Timestep ts)
    {
        if (m_BlockEvents)
            return;

        const float speed = 5.0f;
        const float moveSpeed = speed * ts;

        // FPV Toggle: press F to toggle First-Person View mode (edge-detected)
        bool fPressed = Input::IsKeyPressed(Key::F);
        if (fPressed && !m_FKeyPrevPressed)
        {
            m_FpvEnabled = !m_FpvEnabled;
            // When enabling FPV, initialize the FPV position and orientation from current camera
            if (m_FpvEnabled)
            {
                m_FpvPosition = m_Position;
                m_FpvYaw = glm::degrees(m_Yaw);
                m_FpvPitch = glm::degrees(m_Pitch);
                // Avoid large first-frame mouse delta when entering FPV
                m_InitialMousePosition = glm::vec2(Input::GetMouseX(), Input::GetMouseY());
            }
        }
        m_FKeyPrevPressed = fPressed;

        if (m_FpvEnabled)
        {
            // FPV mouse-look (right mouse) and WASDQE movement
            const glm::vec2 mouse{Input::GetMouseX(), Input::GetMouseY()};
            glm::vec2 delta = (mouse - m_InitialMousePosition) * 0.003f;
            bool rmbPressed = Input::IsMouseButtonPressed(Mouse::ButtonRight);
            if (rmbPressed)
            {
                if (!m_RmbPrevPressed)
                {
                    // RMB was just pressed: initialize and skip applying rotation this frame
                    m_InitialMousePosition = mouse;
                }
                else
                {
                    // Apply rotation when RMB is continuously held
                    float sensitivity = 100.0f;
                    m_FpvYaw += delta.x * sensitivity;
                    // Match editor convention: positive mouse Y -> increase pitch
                    m_FpvPitch += delta.y * sensitivity;
                    if (m_FpvPitch > 89.0f)
                        m_FpvPitch = 89.0f;
                    if (m_FpvPitch < -89.0f)
                        m_FpvPitch = -89.0f;
                    m_InitialMousePosition = mouse;
                }
            }
            m_RmbPrevPressed = rmbPressed;

            // Build orientation quaternion matching UpdateView() (note the negated pitch/yaw)
            glm::quat orientation = glm::quat(glm::vec3(glm::radians(-m_FpvPitch), glm::radians(-m_FpvYaw), 0.0f));

            // Derive basis vectors by rotating local axes so they match view orientation
            glm::vec3 forward = glm::normalize(glm::rotate(orientation, glm::vec3(0.0f, 0.0f, -1.0f)));
            glm::vec3 right = glm::normalize(glm::rotate(orientation, glm::vec3(1.0f, 0.0f, 0.0f)));
            glm::vec3 up = glm::normalize(glm::rotate(orientation, glm::vec3(0.0f, 1.0f, 0.0f)));

            glm::vec3 move(0.0f);
            if (Input::IsKeyPressed(Key::W))
                move += forward;
            if (Input::IsKeyPressed(Key::S))
                move -= forward;
            if (Input::IsKeyPressed(Key::A))
                move -= right;
            if (Input::IsKeyPressed(Key::D))
                move += right;
            if (Input::IsKeyPressed(Key::Q))
                move += up;
            if (Input::IsKeyPressed(Key::E))
                move -= up;

            if (glm::length(move) > 0.0f)
                move = glm::normalize(move);

            float fpvSpeed = 5.0f; // base speed for FPV
            m_FpvPosition += move * fpvSpeed * ts;

            // Update camera position/orientation from FPV state
            m_Position = m_FpvPosition;
            UpdateView();
            return;
        }

        if (Input::IsKeyPressed(Key::LeftAlt))
        {
            const glm::vec2& mouse{Input::GetMouseX(), Input::GetMouseY()};
            glm::vec2 delta = (mouse - m_InitialMousePosition) * 0.003f;
            m_InitialMousePosition = mouse;

            if (Input::IsMouseButtonPressed(Mouse::ButtonRight))
                MousePan(delta);
            else if (Input::IsMouseButtonPressed(Mouse::ButtonLeft))
                MouseRotate(delta);
            else if (Input::IsMouseButtonPressed(Mouse::ButtonMiddle))
                MouseZoom(delta.y);
        }

        UpdateView();
    }

    void EditorCamera::OnEvent(Event& e)
    {
        if (m_BlockEvents)
            return;

        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<MouseScrolledEvent>(TI_BIND_EVENT_FN(EditorCamera::OnMouseScroll));
    }

    bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e)
    {
        float delta = e.GetYOffset() * 0.2f;
        MouseZoom(delta);
        UpdateView();
        return false;
    }

    void EditorCamera::MousePan(const glm::vec2& delta)
    {
        auto [xSpeed, ySpeed] = PanSpeed();
        m_FocalPoint += -GetRightDirection() * delta.x * xSpeed * m_Distance;
        m_FocalPoint += GetUpDirection() * delta.y * ySpeed * m_Distance;
    }

    void EditorCamera::MouseRotate(const glm::vec2& delta)
    {
        float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
        m_Yaw += yawSign * delta.x * RotationSpeed();
        m_Pitch += delta.y * RotationSpeed();
    }

    void EditorCamera::MouseZoom(float delta)
    {
        m_Distance -= delta * ZoomSpeed();
        if (m_Distance < 1.0f)
        {
            m_FocalPoint += GetForwardDirection();
            m_Distance = 1.0f;
        }
    }

    glm::vec3 EditorCamera::GetUpDirection() const
    {
        return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    glm::vec3 EditorCamera::GetRightDirection() const
    {
        return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
    }

    glm::vec3 EditorCamera::GetForwardDirection() const
    {
        return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
    }

    glm::vec3 EditorCamera::CalculatePosition() const
    {
        return m_FocalPoint - GetForwardDirection() * m_Distance;
    }

    glm::quat EditorCamera::GetOrientation() const
    {
        return glm::quat(glm::vec3(-m_Pitch, -m_Yaw, 0.0f));
    }

} // namespace Titan