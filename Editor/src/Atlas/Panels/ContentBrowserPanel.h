#pragma once

#include "Titan/PCH.h"
#include "Titan/Renderer/Texture.h"

namespace Titan
{

    class ContentBrowserPanel
    {
    public:
        ContentBrowserPanel();

        void OnImGuiRender();

    private:
        Ref<Texture2D> GetIconForFile(const std::filesystem::path& filePath);

    private:
        std::filesystem::path m_CurrentDirectory;

        // Icons
        Ref<Texture2D> m_DirectoryIcon;
        Ref<Texture2D> m_DirectoryOpenIcon;
        Ref<Texture2D> m_FileTextIcon;
        Ref<Texture2D> m_FileCodeIcon;
        Ref<Texture2D> m_FileImageIcon;
    };

} // namespace Titan