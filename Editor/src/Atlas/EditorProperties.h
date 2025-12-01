#pragma once

#include "Titan/PCH.h"
#include "Titan/Renderer/SceneRenderer.h"

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

        bool EnableMultiViewports = false;

        // Overlay settings for each viewport (4 viewports)
        OverlaySettings Overlays[4];
    };

    bool LoadEditorProperties(EditorProperties& props, const std::filesystem::path& filepath);
    bool SaveEditorProperties(const EditorProperties& props, const std::filesystem::path& filepath);

} // namespace Titan
