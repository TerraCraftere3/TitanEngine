#pragma once
#include <spdlog/details/log_msg.h>
#include <spdlog/sinks/base_sink.h>
#include <mutex>
#include "Titan/PCH.h"

namespace Titan
{
    struct LogMessage
    {
        std::string Message;
        std::string LoggerName;
        spdlog::level::level_enum Level;
        std::chrono::system_clock::time_point Timestamp;
    };

    class ImGuiLogSink : public spdlog::sinks::base_sink<std::mutex>
    {
    public:
        static ImGuiLogSink& GetInstance();

        const std::vector<LogMessage>& GetMessages() const { return m_Messages; }

        void Clear();

    protected:
        void sink_it_(const spdlog::details::log_msg& msg) override;
        void flush_() override {}

    private:
        ImGuiLogSink() = default;
        std::vector<LogMessage> m_Messages;
    };

} // namespace Titan
