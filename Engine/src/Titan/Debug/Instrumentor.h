#pragma once
#include <imgui.h>
#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>

namespace Titan
{
    class Profiler
    {
    public:
        static Profiler& Get();
        void BeginFrame();
        void EndFrame();

        const std::vector<float>& GetFrameTimes() const { return m_FrameTimes; }

        void AppendPassTime(const std::string& pass, float ms)
        {
            auto& vec = m_PassTimes[pass];
            vec.push_back(ms);

            if (vec.size() > m_MaxSamples)
                vec.erase(vec.begin());
        }

        const std::unordered_map<std::string, std::vector<float>>& GetPassTimes() const { return m_PassTimes; }

        void DrawProfilerUI();

    private:
        using Clock = std::chrono::high_resolution_clock;

        Profiler() : m_MaxSamples(300) {}

        Clock::time_point m_FrameStart;
        std::vector<float> m_FrameTimes;

        std::unordered_map<std::string, std::vector<float>> m_PassTimes;

        size_t m_MaxSamples;
    };

    class ProfilePassTimer
    {
    public:
        ProfilePassTimer(const char* name) : m_Name(name), m_Start(Clock::now()) {}

        ~ProfilePassTimer()
        {
            auto end = Clock::now();
            float ms = std::chrono::duration<float, std::milli>(end - m_Start).count();
            Profiler::Get().AppendPassTime(m_Name, ms);
        }

    private:
        using Clock = std::chrono::high_resolution_clock;

        const char* m_Name;
        Clock::time_point m_Start;
    };

} // namespace Titan

#define TI_PROFILE_BEGIN_FRAME() ::Titan::Profiler::Get().BeginFrame()
#define TI_PROFILE_END_FRAME() ::Titan::Profiler::Get().EndFrame()
#define TI_PROFILE_PASS() ::Titan::ProfilePassTimer ___timer(pass.GetName().c_str())
