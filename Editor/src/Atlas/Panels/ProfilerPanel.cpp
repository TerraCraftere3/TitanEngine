#include "ProfilerPanel.h"

#include <Titan/Debug/Instrumentor.h>
#include <implot.h>

namespace Titan
{
    ProfilerPanel::ProfilerPanel() {}

    void ProfilerPanel::OnImGuiRender(bool* openProfiler)
    {
        ImGui::Begin("Profiler", openProfiler);

        auto s_Scopes = Profiler::GetScopes();
        auto s_Order = Profiler::GetOrder();
        auto s_StartTime = Profiler::GetStartTime();
        constexpr double s_HistoryWindowSeconds = Profiler::GetHistoryWindowSeconds();

        static std::vector<double> values;
        static std::vector<double> ys;
        static std::vector<const char*> labels;
        static int viewMode = 2; // 0: horizontal bars, 1: vertical bars, 2: line

        values.clear();
        ys.clear();
        labels.clear();

        int i = 0;
        for (const char* name : s_Order)
        {
            values.push_back(s_Scopes[name].AverageMs);
            ys.push_back((double)i);
            labels.push_back(name);
            i++;
        }

        if (values.empty())
            return;

        ImGui::TextUnformatted("Profiler View:");
        ImGui::SameLine();
        ImGui::RadioButton("Bars (H)", &viewMode, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Bars (V)", &viewMode, 1);
        ImGui::SameLine();
        ImGui::RadioButton("Line", &viewMode, 2);

        const char* plotTitle = viewMode == 2 ? "Profiler History" : "Profiler Averages";

        if (ImPlot::BeginPlot(plotTitle, ImVec2(-1, 300)))
        {
            const int sampleCount = (int)values.size();

            const double nowSec = std::chrono::duration<double>(std::chrono::steady_clock::now() - s_StartTime).count();
            const double minWindow = nowSec - s_HistoryWindowSeconds;

            if (viewMode == 0)
            {
                ImPlot::SetupAxis(ImAxis_X1, "Avg ms");
                ImPlot::SetupAxis(ImAxis_Y1, nullptr);
                ImPlot::SetupAxisLimits(ImAxis_Y1, -0.5, (double)sampleCount - 0.5, ImGuiCond_Always);
                ImPlot::SetupAxisTicks(ImAxis_Y1, ys.data(), sampleCount, labels.data());

                ImPlot::PlotBars("Avg", values.data(), ys.data(), sampleCount, 0.5, ImPlotBarsFlags_Horizontal);
            }
            else if (viewMode == 1)
            {
                ImPlot::SetupAxis(ImAxis_X1, nullptr);
                ImPlot::SetupAxis(ImAxis_Y1, "Avg ms");
                ImPlot::SetupAxisLimits(ImAxis_X1, -0.5, (double)sampleCount - 0.5, ImGuiCond_Always);
                ImPlot::SetupAxisTicks(ImAxis_X1, ys.data(), sampleCount, labels.data());

                ImPlot::PlotBars("Avg", ys.data(), values.data(), sampleCount, 0.5);
            }
            else
            {
                ImPlot::SetupAxis(ImAxis_X1, "Time (s)");
                ImPlot::SetupAxis(ImAxis_Y1, "Frame ms");
                ImPlot::SetupAxisLimits(ImAxis_X1, minWindow, nowSec, ImGuiCond_Always);
                ImPlot::SetupAxisLimits(ImAxis_Y1, 0.0, 33.33, ImGuiCond_Always); // Fixed max at 33.33ms (30 FPS)

                for (const char* name : s_Order)
                {
                    const auto& history = s_Scopes[name].History;
                    if (history.empty())
                        continue;

                    std::vector<double> xs;
                    std::vector<double> ysLine;
                    xs.reserve(history.size());
                    ysLine.reserve(history.size());
                    for (const ScopeSample& sample : history)
                    {
                        xs.push_back(sample.TimeSec);
                        ysLine.push_back(sample.Ms);
                    }

                    ImPlot::PlotLine(name, xs.data(), ysLine.data(), (int)xs.size());
                }
            }

            ImPlot::EndPlot();
        }
        ImGui::End();
    }
} // namespace Titan