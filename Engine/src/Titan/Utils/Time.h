#pragma once

#include <chrono>
#include "Titan/Core/Timestep.h"

namespace Titan
{
    inline double GetCurrentTimeSeconds()
    {
        using Clock = std::chrono::high_resolution_clock;
        static const auto startTime = Clock::now();
        auto currentTime = Clock::now();
        std::chrono::duration<double> elapsed = currentTime - startTime;
        return elapsed.count();
    }
} // namespace Titan