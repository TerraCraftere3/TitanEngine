#pragma once

#include "Titan/Renderer/Camera.h"

namespace Titan
{

    class TI_API SceneCamera : public Camera
    {
    public:
        SceneCamera();
        virtual ~SceneCamera() = default;

        void SetOrthographic(float size, float nearClip, float farClip);

        void SetViewportSize(uint32_t width, uint32_t height);

        float GetOrthographicSize() const { return m_OrthographicSize; }
        void SetOrthographicSize(float size)
        {
            m_OrthographicSize = size;
            RecalculateProjection();
        }

    private:
        void RecalculateProjection();

    private:
        float m_OrthographicSize = 10.0f;
        float m_OrthographicNear = -1.0f, m_OrthographicFar = FLT_MAX;

        float m_AspectRatio = 0.0f;
    };

} // namespace Titan