#pragma once

#include "Titan/PCH.h"
#include "Titan/Renderer/RHI/Texture.h"
#include "Titan/Scene/Assets.h"

namespace Titan
{

    class ContentBrowserPanel
    {
    public:
        ContentBrowserPanel();

        void OnImGuiRender(bool* openBrowser = nullptr, bool* openFile = nullptr);

    private:
        void RenderBrowser(bool* openBrowser = nullptr);
        void RenderProperties(bool* openFile = nullptr);

    private:
        std::filesystem::path m_CurrentDirectory;
        std::filesystem::path m_Selected;
        AssetType m_SelectedType;
        AssetMeta m_SelectedMeta;
        std::vector<char> m_SelectedBuf = std::vector<char>(1024);
        std::string m_LastSelectedStr;
    };

} // namespace Titan