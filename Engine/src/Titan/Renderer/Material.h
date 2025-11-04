#pragma once

#include "Titan/PCH.h"

namespace Titan
{

    struct Material3D
    {
        glm::vec4 AlbedoColor = glm::vec4(1.0f);
        float Metallic = 0.0f;
        float Roughness = 1.0f;
        float Padding[2];
    };
} // namespace Titan