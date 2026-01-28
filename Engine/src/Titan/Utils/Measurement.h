#pragma once

#include "Titan/Core.h"

namespace Titan{
    class Measurement{
    public:
        virtual void Start() = 0;
        virtual void Stop() = 0;

        virtual double GetElapsedMilliseconds() = 0;
        virtual double GetElapsedSeconds() = 0;

        static Ref<Measurement> CreateCPU();
        static Ref<Measurement> CreateGPU();
    };
};