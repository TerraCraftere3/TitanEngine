#pragma once

#include "Titan/PCH.h"
#include "Titan/Renderer/RHI/Texture.h"
#include "Titan/Scene/Assets.h"

namespace Titan
{

    class ProfilerPanel
    {
    public:
        ProfilerPanel();

        void OnImGuiRender(bool* openProfiler = nullptr);
    };

} // namespace Titan