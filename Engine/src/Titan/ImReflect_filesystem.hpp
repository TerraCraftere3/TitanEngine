#pragma once

#include <imgui.h>
#include <filesystem>
#include <cstring>

#include "ImReflect_entry.hpp"
#include "ImReflect_helper.hpp"
#include "Utils/PlatformUtils.h"

namespace ImReflect
{

    // Type settings for std::filesystem::path
    template <>
    struct type_settings<std::filesystem::path> : ImRequired<std::filesystem::path>
    {
    private:
        const char* _filter = nullptr;
        bool _is_folder_mode = false;
        std::filesystem::path _base_path;
        bool _use_relative = false;

    public:
        type_settings<std::filesystem::path>& filter(const char* filter_str = nullptr)
        {
            _filter = filter_str;
            RETURN_THIS_T(std::filesystem::path);
        }

        type_settings<std::filesystem::path>& folder_mode(const bool v = true)
        {
            _is_folder_mode = v;
            RETURN_THIS_T(std::filesystem::path);
        }

        type_settings<std::filesystem::path>& relative_to(const std::filesystem::path& base_path)
        {
            _base_path = base_path;
            _use_relative = true;
            RETURN_THIS_T(std::filesystem::path);
        }

        const char* get_filter() const { return _filter; }
        bool is_folder_mode() const { return _is_folder_mode; }
        bool is_relative() const { return _use_relative; }
        const std::filesystem::path& get_base_path() const { return _base_path; }
    };

} // namespace ImReflect

// Tag invoke implementation for std::filesystem::path
inline void tag_invoke(ImReflect::ImInput_t, const char* label, std::filesystem::path& value,
                       ImSettings& settings, ImResponse& response)
{
    auto& path_settings = settings.get<std::filesystem::path>();
    auto& path_response = response.get<std::filesystem::path>();

    ImReflect::Detail::scope_id id(label);

    bool changed = false;

    // Static buffer for path input
    static char buffer[512] = "";
    
    // Convert path for display (to absolute if stored as relative)
    std::string display_path = value.string();
    if (path_settings.is_relative() && !value.empty())
    {
        try
        {
            std::filesystem::path abs = path_settings.get_base_path() / value;
            display_path = abs.string();
        }
        catch (const std::exception&)
        {
            // If absolute conversion fails, show as-is
        }
    }
    
    // Copy current path to buffer if different
    if (display_path != buffer)
    {
        std::strncpy(buffer, display_path.c_str(), sizeof(buffer) - 1);
        buffer[sizeof(buffer) - 1] = '\0';
    }

    // Layout: text input + browse button
    ImGui::SetNextItemWidth(-ImGui::GetFrameHeight() - ImGui::GetStyle().ItemInnerSpacing.x - 50.0f); // Leave room for button
    if (ImGui::InputText("##path_input", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
    {
        // Convert absolute path back to relative if needed
        std::string input_path = buffer;
        if (path_settings.is_relative() && !input_path.empty())
        {
            try
            {
                value = std::filesystem::relative(input_path, path_settings.get_base_path());
            }
            catch (const std::exception&)
            {
                value = std::filesystem::path(input_path);
            }
        }
        else
        {
            value = std::filesystem::path(input_path);
        }
        changed = true;
    }

    // Check if input changed but no enter pressed
    if (display_path != std::string(buffer) && ImGui::IsItemDeactivatedAfterEdit())
    {
        std::string input_path = buffer;
        if (path_settings.is_relative() && !input_path.empty())
        {
            try
            {
                value = std::filesystem::relative(input_path, path_settings.get_base_path());
            }
            catch (const std::exception&)
            {
                value = std::filesystem::path(input_path);
            }
        }
        else
        {
            value = std::filesystem::path(input_path);
        }
        changed = true;
    }

    ImGui::SameLine();

    // Browse button
    ImGui::PushButtonRepeat(false);
    if (ImGui::Button("Browse##browse_button", ImVec2(45.0f, 0.0f)))
    {
        std::string result;
        if (path_settings.is_folder_mode())
        {
            // Folder browse - use a generic filter
            result = Titan::FileDialogs::OpenFile(".");
        }
        else
        {
            // File browse - use the specified filter or default
            const char* filter = path_settings.get_filter();
            result = Titan::FileDialogs::OpenFile(filter ? filter : "All Files\0*.*\0");
        }

        if (!result.empty())
        {
            // Convert to relative if needed
            if (path_settings.is_relative())
            {
                try
                {
                    value = std::filesystem::relative(result, path_settings.get_base_path());
                    std::strncpy(buffer, result.c_str(), sizeof(buffer) - 1);
                }
                catch (const std::exception&)
                {
                    value = std::filesystem::path(result);
                    std::strncpy(buffer, result.c_str(), sizeof(buffer) - 1);
                }
            }
            else
            {
                value = std::filesystem::path(result);
                std::strncpy(buffer, result.c_str(), sizeof(buffer) - 1);
            }
            buffer[sizeof(buffer) - 1] = '\0';
            changed = true;
        }
    }
    ImGui::PopButtonRepeat();

    // Display the label
    if (label && label[0] != '\0')
    {
        ImGui::SameLine();
        ImReflect::Detail::text_label(label);
    }

    if (changed)
        path_response.changed();
    ImReflect::Detail::check_input_states(path_response);
}
