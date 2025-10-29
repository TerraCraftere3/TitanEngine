#pragma once

#include "Titan/PCH.h"
#include "Titan/Renderer/Texture.h"
#include "Titan/Scene/Assets.h"

namespace Titan
{

    class ContentBrowserPanel
    {
    public:
        ContentBrowserPanel();

        void OnImGuiRender();

    private:
        void RenderBrowser();
        void RenderProperties();
        Ref<Texture2D> GetIconForFile(const std::filesystem::path& filePath);
        AssetType GetTypeForFile(const std::filesystem::path& filePath);

    private:
        std::filesystem::path m_CurrentDirectory;
        std::filesystem::path m_Selected;
        std::vector<char> m_SelectedBuf = std::vector<char>(1024);
        std::string m_LastSelectedStr;

        // Icons
        Ref<Texture2D> m_DirectoryIcon;
        Ref<Texture2D> m_DirectoryOpenIcon;
        Ref<Texture2D> m_FileTextIcon;
        Ref<Texture2D> m_FileCodeIcon;
        Ref<Texture2D> m_FileImageIcon;
        Ref<Texture2D> m_FileMaterialIcon;
    };

} // namespace Titan