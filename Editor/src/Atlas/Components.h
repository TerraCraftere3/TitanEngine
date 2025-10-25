#pragma once
#include "Titan/PCH.h"

namespace Titan
{

    class Component
    {
    public:
        static void Vec4Control(const std::string& label, glm::vec4& values, float resetValue = 0.0f,
                                float columnWidth = 100.0f);
        static void Vec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f,
                                float columnWidth = 100.0f);
        static void Vec2Control(const std::string& label, glm::vec2& values, float resetValue = 0.0f,
                                float columnWidth = 100.0f);
    };

} // namespace Titan