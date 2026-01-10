#include "Instrumentor.h"

#include <imgui.h>
#include <implot.h>
#include <cfloat>

namespace Titan
{
    std::unordered_map<std::string, ScopeStats> Profiler::s_Scopes;
    std::vector<const char*> Profiler::s_Order;
    std::chrono::steady_clock::time_point Profiler::s_StartTime = std::chrono::steady_clock::now();

    void Profiler::BeginFrame()
    {
        for (auto& [_, stats] : s_Scopes)
            stats.CurrentMs = 0.0;
    }

    void Profiler::EndFrame() {}

    void Profiler::AddSample(const char* name, double ms)
    {
        const double nowSec = std::chrono::duration<double>(std::chrono::steady_clock::now() - s_StartTime).count();
        auto& stats = s_Scopes[name];

        if (stats.SampleCount == 0)
            s_Order.push_back(name);

        stats.CurrentMs += ms;
        stats.SampleCount++;

        stats.History.push_back({nowSec, ms});
        while (!stats.History.empty() && stats.History.front().TimeSec < nowSec - s_HistoryWindowSeconds)
            stats.History.pop_front();

        // Running average
        stats.AverageMs += (ms - stats.AverageMs) / (double)stats.SampleCount;
    }

    void Profiler::Plot()
    {
        static std::vector<double> values;
        static std::vector<double> ys;
        static std::vector<const char*> labels;
        static int viewMode = 0; // 0: horizontal bars, 1: vertical bars, 2: line

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

                // First pass: calculate mean and standard deviation for Y axis
                double sum = 0.0;
                int sampleTotal = 0;

                for (const char* name : s_Order)
                {
                    const auto& history = s_Scopes[name].History;
                    if (history.empty())
                        continue;

                    for (const ScopeSample& sample : history)
                    {
                        sum += sample.Ms;
                        sampleTotal++;
                    }
                }

                // Calculate standard deviation
                double mean = sampleTotal > 0 ? sum / sampleTotal : 0.0;
                double sumSqDiff = 0.0;

                for (const char* name : s_Order)
                {
                    const auto& history = s_Scopes[name].History;
                    if (history.empty())
                        continue;

                    for (const ScopeSample& sample : history)
                    {
                        double diff = sample.Ms - mean;
                        sumSqDiff += diff * diff;
                    }
                }

                double stdDev = sampleTotal > 1 ? std::sqrt(sumSqDiff / (sampleTotal - 1)) : 0.0;
                double padding = stdDev > 0.0 ? stdDev * 2.0 : mean * 0.5;

                // Set Y limits before plotting
                if (sampleTotal > 0)
                {
                    ImPlot::SetupAxisLimits(ImAxis_Y1, std::max(0.0, mean - padding), mean + padding, ImGuiCond_Always);
                }

                // Second pass: plot the lines
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
    }

    /* =======================
       ScopeTimer
       ======================= */

    ScopeTimer::ScopeTimer(const char* name) : m_Name(name), m_Start(std::chrono::high_resolution_clock::now()) {}

    ScopeTimer::~ScopeTimer()
    {
        auto end = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(end - m_Start).count();
        Profiler::AddSample(m_Name, ms);
    }
} // namespace Titan
