#include "Titan/Core/Log.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include "Titan/Core/ImGuiLogSink.h"
#include "Titan/PCH.h"

namespace Titan
{
    std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
    std::shared_ptr<spdlog::logger> Log::s_ClientLogger;

    void Log::Init()
    {
        std::string format = "[%T] [%^%l%$] %n: %v";
        std::string miniFormat = "[%^%l%$] %n: %v";
        spdlog::level::level_enum level = spdlog::level::trace;

        // Create sinks
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(level);
        console_sink->set_pattern(format);

        // Use shared_ptr wrapper around the singleton instance
        std::shared_ptr<ImGuiLogSink> imgui_sink(&ImGuiLogSink::GetInstance(), [](ImGuiLogSink*) {});
        imgui_sink->set_level(level);
        imgui_sink->set_pattern(miniFormat);

        // Core Logger with both sinks
        std::vector<spdlog::sink_ptr> core_sinks = {console_sink, imgui_sink};
        s_CoreLogger = std::make_shared<spdlog::logger>("CORE", core_sinks.begin(), core_sinks.end());
        s_CoreLogger->set_level(level);
        spdlog::register_logger(s_CoreLogger);

        // Client Logger with both sinks
        std::vector<spdlog::sink_ptr> client_sinks = {console_sink, imgui_sink};
        s_ClientLogger = std::make_shared<spdlog::logger>("APP", client_sinks.begin(), client_sinks.end());
        s_ClientLogger->set_level(level);
        spdlog::register_logger(s_ClientLogger);
    }
} // namespace Titan