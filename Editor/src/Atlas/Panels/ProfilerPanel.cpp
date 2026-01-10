#include "ProfilerPanel.h"

#include <Titan/Debug/Instrumentor.h>

namespace Titan{
    ProfilerPanel::ProfilerPanel()
    {
    }

    void ProfilerPanel::OnImGuiRender(bool* openProfiler)
    {
        ImGui::Begin("Profiler", openProfiler);
        Profiler::Plot();
        ImGui::End();
    }
}