#include "Log.h"
#include "PCH.h"

namespace Titan
{
    std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
    std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

    void Log::Init()
    {
        std::string format = "[%T] [%^%l%$] %n: %v";
        spdlog::level::level_enum level = spdlog::level::trace;

        // Core Logger
        s_CoreLogger = spdlog::stdout_color_mt("CORE");
        s_CoreLogger->set_level(level);
        s_CoreLogger->set_pattern(format);

        // Client Logger
        s_ClientLogger = spdlog::stdout_color_mt("APP");
        s_ClientLogger->set_level(level);
        s_ClientLogger->set_pattern(format);
    }
} // namespace Titan