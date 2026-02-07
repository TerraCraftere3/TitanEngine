#include "Titan/Core/Log.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include "Titan/Core/ImGuiLogSink.h"
#include "Titan/PCH.h"

namespace Titan
{

    class SPDLogLogger : public Logger
    {
    public:
        SPDLogLogger(std::string name)
        {
            std::string format = "[%T] [%^%l%$] %n: %v";
            std::string miniFormat = "[%^%l%$] %n: %v";
            spdlog::level::level_enum level = spdlog::level::trace;

            // Create sinks
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            console_sink->set_level(level);
            console_sink->set_pattern(format);

            // Use shared_ptr wrapper around the singleton instance
            Ref<ImGuiLogSink> imgui_sink(&ImGuiLogSink::GetInstance(), [](ImGuiLogSink*) {});
            imgui_sink->set_level(level);
            imgui_sink->set_pattern(miniFormat);

            std::vector<spdlog::sink_ptr> sinks = {console_sink, imgui_sink};
            m_Logger = CreateRef<spdlog::logger>(name, sinks.begin(), sinks.end());
            m_Logger->set_level(level);
            spdlog::register_logger(m_Logger);
        }

        void LogInternal(LogLevel level, fmt::string_view fmt, fmt::format_args args) override
        {
            m_Logger->log(static_cast<spdlog::level::level_enum>(level), fmt::vformat(fmt, args));
        }

    private:
        Ref<spdlog::logger> m_Logger;
    };

    Ref<Logger> Log::s_CoreLogger;
    Ref<Logger> Log::s_ClientLogger;

    void Log::Init()
    {
        s_CoreLogger = CreateRef<SPDLogLogger>("CORE");
        s_ClientLogger = CreateRef<SPDLogLogger>("APP");
    }
} // namespace Titan