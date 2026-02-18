#include "ProjectConfigWindow.h"

#include <Titan/Project/Project.h>
#include <Titan/Utils/PlatformUtils.h>
#include <ImReflect.hpp>
#include <Titan/ImReflect_filesystem.hpp>

namespace Titan
{

    void ProjectConfigWindow::OnImGuiRender()
    {
        if (!m_Open)
            return;
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
        ImGui::Begin("Project Config", &m_Open, window_flags);

        Ref<Project> activeProject = Project::GetActive();
        ProjectConfig& config = activeProject->GetConfig();

        ImReflect::Input("Project Name", &config.Name);
        {
            auto settings = ImSettings();
            settings.push<std::filesystem::path>()
                .folder_mode()
                .relative_to(activeProject->GetProjectDirectory())
                .pop();

            ImReflect::Input("Asset Directory", &config.AssetDirectory, settings);
        }
        {
            auto settings = ImSettings();
            settings.push<std::filesystem::path>()
                .filter("Scene Files (*.titan)\0*.titan\0All Files (*.*)\0*.*\0")
                .relative_to(activeProject->GetAssetDirectory())
                .pop();

            ImReflect::Input("Start Scene Path", &config.StartScene, settings);
        }
        {
            auto settings = ImSettings();
            settings.push<std::filesystem::path>()
                .filter("Script Module (*.dll)\0*.dll\0All Files (*.*)\0*.*\0")
                .relative_to(activeProject->GetAssetDirectory())
                .pop();

            ImReflect::Input("Script Module Path", &config.ScriptModulePath, settings);
        }

        if (ImGui::Button("Save"))
        {
            std::filesystem::path projectPath = Project::GetProjectDirectory() / "project.tiproj";
            if (Project::SaveActive(projectPath))
            {
                TI_CORE_INFO("Project config saved successfully.");
                Process::RestartWithSameArgs();
            }
            else
            {
                TI_CORE_ERROR("Failed to save project config.");
            }
        }

        ImGui::End();
    }

} // namespace Titan