#include "Titan/Core/ImGuiLogSink.h"
#include "Titan/PCH.h"

namespace Titan
{
    ImGuiLogSink& ImGuiLogSink::GetInstance()
    {
        static ImGuiLogSink instance;
        return instance;
    }

    void ImGuiLogSink::Clear()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        m_Messages.clear();
    }

    void ImGuiLogSink::sink_it_(const spdlog::details::log_msg& msg)
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

} // namespace Titan
