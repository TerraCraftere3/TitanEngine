#include "OpenGLMeasurement.h"

namespace Titan{
    OpenGLMeasurement::OpenGLMeasurement()
        : isRunning(false)
    {
        glGenQueries(2, queryID);
    }

    OpenGLMeasurement::~OpenGLMeasurement()
    {
        glDeleteQueries(2, queryID);
    }

    void OpenGLMeasurement::Start()
    {
        if (isRunning)
            return;

        glBeginQuery(GL_TIME_ELAPSED, queryID[0]);
        isRunning = true;
    }

    void OpenGLMeasurement::Stop()
    {
        if (!isRunning)
            return;

        glEndQuery(GL_TIME_ELAPSED);
        glBeginQuery(GL_TIME_ELAPSED, queryID[1]);
        glEndQuery(GL_TIME_ELAPSED);
        isRunning = false;
    }

    double OpenGLMeasurement::GetElapsedMilliseconds()
    {
        GLuint64 timeElapsed = 0;
        glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &timeElapsed);
        return static_cast<double>(timeElapsed) / 1e6; // Convert nanoseconds to milliseconds
    }

    double OpenGLMeasurement::GetElapsedSeconds()
    {
        GLuint64 timeElapsed = 0;
        glGetQueryObjectui64v(queryID[0], GL_QUERY_RESULT, &timeElapsed);
        return static_cast<double>(timeElapsed) / 1e9; // Convert nanoseconds to seconds
    }
}