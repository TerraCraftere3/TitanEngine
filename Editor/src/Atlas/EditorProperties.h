#pragma once

#include <Titan/PCH.h>

namespace Titan
{
    struct EditorProperties
    {
        // Panels
        bool ShowSceneHierarchy = true;
        bool ShowSceneHierarchyProperties = true;

        bool ShowContentBrowser = true;
        bool ShowContentBrowserFile = true;

        bool ShowStatistics = true;
        bool ShowViewport = true;
        bool ShowProfiler = true;
        bool ShowLog = true;
    };

    // Loads editor properties from a YAML configuration file.
    // Returns true if load succeeded (file existed and parsed), false otherwise (defaults remain).
    bool LoadEditorProperties(EditorProperties& props, const std::filesystem::path& filepath);

    // Saves editor properties to a YAML configuration file. Returns true on success.
    bool SaveEditorProperties(const EditorProperties& props, const std::filesystem::path& filepath);

} // namespace Titan
