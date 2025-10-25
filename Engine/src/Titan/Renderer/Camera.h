#pragma once

#include "Titan/Core.h"
#include "Titan/PCH.h"

namespace Titan
{
    class TI_API Camera
    {
    public:
        Camera() = default;
        Camera(const glm::mat4& projection) : m_ProjectionMatrix(projection) {}
        virtual ~Camera() = default;

        const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }

    protected:
        glm::mat4 m_ProjectionMatrix{1.0f};
    };

} // namespace Titan