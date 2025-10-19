#pragma once

#include "Titan/PCH.h"

namespace Titan
{

    class OrthographicCamera
    {
    public:
        OrthographicCamera(float left, float right, float bottom, float top);

        const glm::mat4& GetProjectionMatrix() { return m_ProjectionMatrix; }
        const glm::mat4& GetViewMatrix() { return m_ViewMatrix; }
        const glm::mat4& GetViewProjectionMatrix() { return m_ViewProjectionMatrix; }
        const glm::vec3& GetPosition() const { return m_Position; }
        const glm::vec3& GetRotation() const { return m_Rotation; }
        float GetZRotation() const { return m_Rotation.z; }

        void SetPosition(const glm::vec3& position)
        {
            m_Position = position;
            RecalculateViewMatrix();
        }
        void SetRotation(const glm::vec3& rotation)
        {
            m_Rotation = rotation;
            RecalculateViewMatrix();
        }
        void SetRotation(float rotationZ)
        {
            m_Rotation.z = rotationZ;
            RecalculateViewMatrix();
        }

    private:
        void RecalculateViewMatrix();

    private:
        glm::mat4 m_ProjectionMatrix;
        glm::mat4 m_ViewMatrix;
        glm::mat4 m_ViewProjectionMatrix;

        glm::vec3 m_Position;
        glm::vec3 m_Rotation;
    };

} // namespace Titan