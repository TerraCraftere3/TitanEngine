#include "LogPanel.h"
#include <Titan/FontAwesome7.h>
#include <imgui.h>

namespace Titan
{
    LogPanel::LogPanel()
    {
    }

    void LogPanel::OnImGuiRender(bool* open)
    {
        ImGui::Begin(ICON_FA_TERMINAL " Log###Log", open);

        // Toolbar
        if (ImGui::Button("Clear"))
            ImGuiLogSink::GetInstance().Clear();
        
        ImGui::SameLine();
        ImGui::Checkbox("Auto-scroll", &m_AutoScroll);
        
        ImGui::SameLine();
        ImGui::Checkbox("Timestamp", &m_ShowTimestamp);

        ImGui::Separator();

        // Log display
        ImGui::BeginChild("LogScrollRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        const auto& messages = ImGuiLogSink::GetInstance().GetMessages();
        
        for (const auto& msg : messages)
        {
            // Filter: Show only errors and warnings from CORE, but everything from APP
            bool shouldShow = false;
            if (msg.LoggerName == "CORE")
            {
                // Only show errors and warnings from core
                if (msg.Level >= spdlog::level::warn)
                    shouldShow = true;
            }
            else if (msg.LoggerName == "APP")
            {
                // Show everything from App
                shouldShow = true;
            }

            if (!shouldShow)
                continue;

            // Color based on level
            ImVec4 color;
            switch (msg.Level)
            {
                case spdlog::level::trace:
                    color = ImVec4(0.5f, 0.5f, 0.5f, 1.0f); // Gray
                    break;
                case spdlog::level::debug:
                    color = ImVec4(0.0f, 0.7f, 1.0f, 1.0f); // Cyan
                    break;
                case spdlog::level::info:
                    color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // Green
                    break;
                case spdlog::level::warn:
                    color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
                    break;
                case spdlog::level::err:
                    color = ImVec4(1.0f, 0.3f, 0.3f, 1.0f); // Red
                    break;
                case spdlog::level::critical:
                    color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Bright Red
                    break;
                default:
                    color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // White
                    break;
            }

            ImGui::PushStyleColor(ImGuiCol_Text, color);
            
            if (m_ShowTimestamp)
            {
                auto time = std::chrono::system_clock::to_time_t(msg.Timestamp);
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(msg.Timestamp.time_since_epoch()) % 1000;
                
                std::tm tm_buf;
                localtime_s(&tm_buf, &time);
                
                ImGui::Text("[%02d:%02d:%02d.%03lld] ", 
                    tm_buf.tm_hour, tm_buf.tm_min, tm_buf.tm_sec, ms.count());
                ImGui::SameLine();
            }
            
            ImGui::TextUnformatted(msg.Message.c_str());
            ImGui::PopStyleColor();
        }

        if (m_AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);

        ImGui::EndChild();

        ImGui::End();
    }

} // namespace Titan
