#pragma once

#include <string>
#include "Titan/Core.h"

namespace Titan
{
    struct Physics2DMaterial
    {
        float Density = 1.0f;
        float Friction = 0.5f;
        float Restitution = 0.0f;
        float RestitutionThreshold = 0.5f;

        std::string SourcePath = "";

        Physics2DMaterial() = default;
        Physics2DMaterial(const Physics2DMaterial&) = default;

        void TI_API Save();
        static Ref<Physics2DMaterial> TI_API Create(const std::string& path);
    };

} // namespace Titan