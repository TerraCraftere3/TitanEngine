#include "EditorProperties.h"

#include <Titan/PCH.h>
#include <Titan/Utils/PlatformUtils.h>
#include <yaml-cpp/yaml.h>

namespace Titan
{
    bool LoadEditorProperties(EditorProperties& props, const std::filesystem::path& filepath)
    {
        if (!std::filesystem::exists(filepath))
            return false;

        YAML::Node data;
        try
        {
            data = YAML::LoadFile(filepath.string());
        }
        catch (YAML::ParserException& e)
        {
            TI_CORE_WARN("Failed to parse Editor Config '{}': {}", filepath.string(), e.what());
            return false;
        }

        auto panels = data["Panels"];
        if (panels)
        {
            // clang-format off
            if (panels["ShowSceneHierarchy"])           props.ShowSceneHierarchy = panels["ShowSceneHierarchy"].as<bool>();
            if (panels["ShowSceneHierarchyProperties"]) props.ShowSceneHierarchyProperties = panels["ShowSceneHierarchyProperties"].as<bool>();
            if (panels["ShowContentBrowser"])           props.ShowContentBrowser = panels["ShowContentBrowser"].as<bool>();
            if (panels["ShowContentBrowserFile"])       props.ShowContentBrowserFile = panels["ShowContentBrowserFile"].as<bool>();
            if (panels["ShowStatistics"])               props.ShowStatistics = panels["ShowStatistics"].as<bool>();
            if (panels["ShowViewport"])                 props.ShowViewport = panels["ShowViewport"].as<bool>();
            if (panels["ShowProfiler"])                 props.ShowProfiler = panels["ShowProfiler"].as<bool>();
            if (panels["ShowLog"])                      props.ShowLog = panels["ShowLog"].as<bool>();
            // clang-format on
        }

        TI_CORE_INFO("Loaded Editor Properties from '{}'", filepath.string());
        return true;
    }

    bool SaveEditorProperties(const EditorProperties& props, const std::filesystem::path& filepath)
    {
        // Ensure directory exists
        std::error_code ec;
        std::filesystem::create_directories(filepath.parent_path(), ec);
        if (ec)
        {
            TI_CORE_WARN("Failed to create config directory '{}': {}", filepath.parent_path().string(), ec.message());
        }

        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Panels" << YAML::Value;
        out << YAML::BeginMap;
        // clang-format off
        out << YAML::Key << "ShowSceneHierarchy" << YAML::Value <<           props.ShowSceneHierarchy;
        out << YAML::Key << "ShowSceneHierarchyProperties" << YAML::Value << props.ShowSceneHierarchyProperties;
        out << YAML::Key << "ShowContentBrowser" << YAML::Value <<           props.ShowContentBrowser;
        out << YAML::Key << "ShowContentBrowserFile" << YAML::Value <<       props.ShowContentBrowserFile;
        out << YAML::Key << "ShowStatistics" << YAML::Value <<               props.ShowStatistics;
        out << YAML::Key << "ShowViewport" << YAML::Value <<                 props.ShowViewport;
        out << YAML::Key << "ShowProfiler" << YAML::Value <<                 props.ShowProfiler;
        out << YAML::Key << "ShowLog" << YAML::Value <<                      props.ShowLog;
        // clang-format on
        out << YAML::EndMap;
        out << YAML::EndMap;

        std::ofstream fout(filepath);
        if (!fout.is_open())
        {
            TI_CORE_ERROR("Failed to open Editor Config file for write: '{}'", filepath.string());
            return false;
        }
        fout << out.c_str();
        TI_CORE_INFO("Saved Editor Properties to '{}'", filepath.string());
        return true;
    }
} // namespace Titan
