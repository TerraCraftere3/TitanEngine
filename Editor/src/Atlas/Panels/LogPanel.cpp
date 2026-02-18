#include "LogPanel.h"
#include <Titan/Vendor/FontAwesome7.h>
#include <imgui.h>

namespace Titan
{
    LogPanel::LogPanel() {}

    void LogPanel::OnImGuiRender(bool* open)
    {
        TI_PROFILE_FUNCTION();
        ImGui::Begin(ICON_FA_TERMINAL " Log###Log", open);

        ImGui::SameLine();
        ImGui::Checkbox("Auto-scroll", &m_AutoScroll);

        ImGui::SameLine();
        ImGui::Checkbox("Timestamp", &m_ShowTimestamp);

        ImGui::SameLine();
        ImGui::Checkbox("Only Important", &m_ShowOnlyErrors);

        ImGui::Separator();

        // Log display
        ImGui::BeginChild("LogScrollRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        const auto& messages = Log::GetMessages();

        int visibleIndex = 0;
        for (const auto& msg : messages)
        {
            bool shouldShow = false;
            if (msg.LoggerName == "CORE")
            {
                if (m_ShowOnlyErrors)
                    shouldShow = (msg.Level >= spdlog::level::warn);
                else
                    shouldShow = (msg.Level > spdlog::level::trace);
            }
            else if (msg.LoggerName == "APP")
            {
                if (m_ShowOnlyErrors)
                    shouldShow = (msg.Level >= spdlog::level::warn);
                else
                    shouldShow = (msg.Level > spdlog::level::trace);
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

            std::string displayText;
            if (m_ShowTimestamp)
            {
                auto time = std::chrono::system_clock::to_time_t(msg.Timestamp);
                auto ms =
                    std::chrono::duration_cast<std::chrono::milliseconds>(msg.Timestamp.time_since_epoch()) % 1000;

                std::tm tm_buf;
                localtime_s(&tm_buf, &time);

                char timeBuffer[32];
                snprintf(timeBuffer, sizeof(timeBuffer), "[%02d:%02d:%02d.%03lld] ", tm_buf.tm_hour, tm_buf.tm_min,
                         tm_buf.tm_sec, static_cast<long long>(ms.count()));

                displayText = timeBuffer;
            }

            displayText += msg.Message;

            ImGui::PushID(visibleIndex);
            ImGui::PushStyleColor(ImGuiCol_Text, color);

            const bool isSelected = (m_SelectedIndex == visibleIndex);
            if (ImGui::Selectable(displayText.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns))
                m_SelectedIndex = visibleIndex;

            ImGui::PopStyleColor();
            ImGui::PopID();
            ++visibleIndex;
        }

        if (m_AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);

        ImGui::EndChild();

        ImGui::End();
    }

} // namespace Titan
