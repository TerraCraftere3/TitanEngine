#pragma once

#include "Titan/PCH.h"
#include "Titan/Renderer/Texture.h"
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