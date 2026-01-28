#pragma once

#include "Titan/Utils/Measurement.h"

// clang-format off
#ifdef APIENTRY
    #undef APIENTRY
#endif
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

namespace Titan
{
    class OpenGLMeasurement : public Measurement
    {
    public:
        OpenGLMeasurement();
        virtual ~OpenGLMeasurement();

        virtual void Start() override;
        virtual void Stop() override;

        virtual double GetElapsedMilliseconds() override;
        virtual double GetElapsedSeconds() override;

    private:
        GLuint queryID[2];
        bool isRunning;
    };
} // namespace Titan