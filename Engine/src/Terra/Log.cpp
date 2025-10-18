#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace Terra
{
    std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
    std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

    void Log::Init()
    {
        // Core Logger
        s_CoreLogger = spdlog::stdout_color_mt("CORE");
        s_CoreLogger->set_level(spdlog::level::trace);
        s_CoreLogger->set_pattern("[%T] [%^%l%$] %v");

        // Client Logger
        s_ClientLogger = spdlog::stdout_color_mt("APP");
        s_ClientLogger->set_level(spdlog::level::trace);
        s_ClientLogger->set_pattern("[%T] [%^%l%$] %v");
    }
} // namespace Terra