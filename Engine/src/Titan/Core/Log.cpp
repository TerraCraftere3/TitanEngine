#include "Titan/Core/Log.h"
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <mutex>
#include "Titan/PCH.h"

namespace Titan
{
    class CustomSink : public spdlog::sinks::base_sink<std::mutex>
    {
    public:
        static CustomSink& GetInstance()
        {
            static CustomSink instance;
            return instance;
        }

        const std::vector<LogMessage>& GetMessages() const { return m_Messages; }

        void Clear()
        {
            std::lock_guard<std::mutex> lock(mutex_);
            m_Messages.clear();
        }

    protected:
        void sink_it_(const spdlog::details::log_msg& msg) override
        {
            spdlog::memory_buf_t formatted;
            formatter_->format(msg, formatted);

            LogMessage logMsg;
            logMsg.Message = std::string(formatted.data(), formatted.size());
            logMsg.LoggerName = std::string(msg.logger_name.data(), msg.logger_name.size());
            logMsg.Level = msg.level;
            logMsg.Timestamp = msg.time;

            m_Messages.push_back(logMsg);

            // Keep only the last 1000 messages to prevent memory issues
            if (m_Messages.size() > 1000)
                m_Messages.erase(m_Messages.begin());
        }
        void flush_() override {}

    private:
        CustomSink() = default;
        std::vector<LogMessage> m_Messages;
    };

    class SPDLogLogger : public Logger
    {
    public:
        SPDLogLogger(std::string name)
        {
            std::string format = "[%T] [%^%l%$] %n: %v";
            std::string miniFormat = "%v";
            spdlog::level::level_enum level = spdlog::level::trace;

            // Create sinks
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
#ifdef TI_ENABLE_LOGGING
            console_sink->set_level(level);
#else
            console_sink->set_level(spdlog::level::err);
#endif
            console_sink->set_pattern(format);

            // Use shared_ptr wrapper around the singleton instance
            Ref<CustomSink> custom_sink(&CustomSink::GetInstance(), [](CustomSink*) {});
            custom_sink->set_level(level);
            custom_sink->set_pattern(miniFormat);

            std::vector<spdlog::sink_ptr> sinks = {console_sink, custom_sink};
            m_Logger = CreateRef<spdlog::logger>(name, sinks.begin(), sinks.end());
            spdlog::register_logger(m_Logger);
        }

        void LogInternal(LogLevel level, fmt::string_view fmt, fmt::format_args args) override
        {
            spdlog::level::level_enum spdlogLevel = spdlog::level::info;
            switch (level)
            {
                case LogLevel::Trace:
                    spdlogLevel = spdlog::level::trace;
                    break;
                case LogLevel::Info:
                    spdlogLevel = spdlog::level::info;
                    break;
                case LogLevel::Warn:
                    spdlogLevel = spdlog::level::warn;
                    break;
                case LogLevel::Error:
                    spdlogLevel = spdlog::level::err;
                    break;
                case LogLevel::Critical:
                    spdlogLevel = spdlog::level::critical;
                    break;
                default:
                    spdlogLevel = spdlog::level::info;
                    break;
            }
            m_Logger->log(spdlogLevel, fmt::vformat(fmt, args));
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

    std::vector<LogMessage> Log::GetMessages()
    {
        return CustomSink::GetInstance().GetMessages();
    }
} // namespace Titan