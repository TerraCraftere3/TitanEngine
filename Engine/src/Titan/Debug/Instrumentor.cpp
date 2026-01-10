#include "Instrumentor.h"

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
