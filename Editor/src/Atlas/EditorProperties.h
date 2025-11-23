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
    };

} // namespace Titan
