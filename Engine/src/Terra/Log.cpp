#include "Log.h"

namespace Terra
{
    std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
    std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

    void Log::Init()
    {
        s_CoreLogger = spdlog::stdout_color_mt("CORE");
        s_CoreLogger->set_level(spdlog::level::trace);
        s_CoreLogger->set_pattern("[%T] [%^%l%$] %v");
        s_ClientLogger = spdlog::stdout_color_mt("APP");
        s_ClientLogger->set_level(spdlog::level::trace);
        s_CoreLogger->set_pattern("[%T] [%^%l%$] %v");
    }
} // namespace Terra