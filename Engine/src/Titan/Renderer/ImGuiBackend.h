#pragma once
#include "imgui.h"
#ifndef IMGUI_DISABLE

namespace Titan
{

    IMGUI_IMPL_API bool ImGui_ImplTitan_Init();
    IMGUI_IMPL_API void ImGui_ImplTitan_Shutdown();
    IMGUI_IMPL_API void ImGui_ImplTitan_NewFrame();
    IMGUI_IMPL_API void ImGui_ImplTitan_RenderDrawData(ImDrawData* draw_data);
}; // namespace Titan
#endif