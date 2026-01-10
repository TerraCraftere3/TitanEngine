#pragma once

#include <chrono>
#include <deque>
#include <string>
#include <unordered_map>
#include <vector>

namespace Titan
{
    struct ScopeSample
    {
        double TimeSec;
        double Ms;
    };

    struct ScopeStats
    {
        double CurrentMs = 0.0;
        double AverageMs = 0.0;
        uint64_t SampleCount = 0;
        std::deque<ScopeSample> History;
    };

    class Profiler
    {
    public:
        static void BeginFrame();
        static void EndFrame();
        static void AddSample(const char* name, double ms);
        static void Plot();

    private:
        static std::unordered_map<std::string, ScopeStats> s_Scopes;
        static std::vector<const char*> s_Order;
        inline static constexpr double s_HistoryWindowSeconds = 5.0;
        static std::chrono::steady_clock::time_point s_StartTime;
    };

    class ScopeTimer
    {
    public:
        ScopeTimer(const char* name);
        ~ScopeTimer();

    private:
        const char* m_Name;
        std::chrono::high_resolution_clock::time_point m_Start;
    };
} // namespace Titan

/* =======================
   Macros
   ======================= */

#define TI_PROFILE_BEGIN_FRAME() ::Titan::Profiler::BeginFrame()
#define TI_PROFILE_END_FRAME() ::Titan::Profiler::EndFrame()
#define TI_PROFILE_SCOPE(name) ::Titan::ScopeTimer TI_SCOPE_##__LINE__(name)
#define TI_PROFILE_FUNCTION() TI_PROFILE_SCOPE(__FUNCTION__)