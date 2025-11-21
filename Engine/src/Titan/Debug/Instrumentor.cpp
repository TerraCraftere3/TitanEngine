#include "Instrumentor.h"

#include <imgui.h>

namespace Titan
{
    Profiler& Profiler::Get()

    {
        static Profiler instance;
        return instance;
    }
    void Profiler::BeginFrame()
    {
        m_FrameStart = Clock::now();
    }

    void Profiler::EndFrame()
    {
        auto end = Clock::now();
        float ms = std::chrono::duration<float, std::milli>(end - m_FrameStart).count();

        m_FrameTimes.push_back(ms);
        if (m_FrameTimes.size() > m_MaxSamples)
            m_FrameTimes.erase(m_FrameTimes.begin());
    }

    void Profiler::DrawProfilerUI()
    {
        ImGui::Begin("Profiler");

        // Frame time graph
        if (!m_FrameTimes.empty())
        {
            float avg = 0.0f;
            for (float t : m_FrameTimes)
                avg += t;
            avg /= m_FrameTimes.size();

            ImGui::Text("Frame %.2f ms (%.1f FPS)", m_FrameTimes.back(), 1000.0f / m_FrameTimes.back());
            ImGui::Text("Avg:   %.2f ms (%.1f FPS)", avg, 1000.0f / avg);

            ImGui::PlotLines("Frame Time", m_FrameTimes.data(), (int)m_FrameTimes.size(), 0, nullptr, 0.0f, 40.0f,
                             ImVec2(0, 100));
        }

        ImGui::Separator();
        ImGui::Text("Render Passes");

        // Each pass
        for (const auto& [name, times] : m_PassTimes)
        {
            if (times.empty())
                continue;

            ImGui::Text("%s (%.2f ms)", name.c_str(), times.back());

            ImGui::PlotLines(("##" + name).c_str(), times.data(), (int)times.size(), 0, nullptr, 0.0f, 10.0f,
                             ImVec2(0, 60));
        }

        ImGui::End();
    }
} // namespace Titan