#include "ContentBrowserPanel.h"

namespace Titan
{
    static const std::filesystem::path s_AssetPath = "assets";

    ContentBrowserPanel::ContentBrowserPanel() : m_CurrentDirectory(s_AssetPath)
    {
        m_DirectoryIcon = Texture2D::Create("resources/icons/contentbrowser/folder.svg");
        m_DirectoryOpenIcon = Texture2D::Create("resources/icons/contentbrowser/folder-open.svg");
        m_FileTextIcon = Texture2D::Create("resources/icons/contentbrowser/file-text.svg");
    }

    void ContentBrowserPanel::OnImGuiRender()
    {
        ImGui::Begin("Content Browser");

        try
        {
            if (m_CurrentDirectory != std::filesystem::path(s_AssetPath))
            {
                if (ImGui::Button("<-"))
                {
                    m_CurrentDirectory = m_CurrentDirectory.parent_path();
                }
            }

            static float padding = 28.0f;
            static float thumbnailSize = 128.0f;
            float cellSize = thumbnailSize + padding;

            float panelWidth = ImGui::GetContentRegionAvail().x;
            int columnCount = (int)(panelWidth / cellSize);
            if (columnCount < 1)
                columnCount = 1;

            ImGui::Columns(columnCount, 0, false);

            for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
            {
                const auto& path = directoryEntry.path();
                auto relativePath = std::filesystem::relative(path, s_AssetPath);
                std::string filenameString = relativePath.filename().string();

                Ref<Texture2D> icon = GetIconForFile(path);
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                ImGui::ImageButton(filenameString.c_str(), icon->GetNativeTexture(), {thumbnailSize, thumbnailSize},
                                   {0, 1}, {1, 0});
                ImGui::PopStyleColor();

                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    if (directoryEntry.is_directory())
                        m_CurrentDirectory /= path.filename();
                }

                ImGui::TextWrapped(filenameString.c_str());
                ImGui::NextColumn();
            }
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            ImGui::Text("Filesystem Error: {}", e.what());
        }

        ImGui::End();
    }

    Ref<Texture2D> ContentBrowserPanel::GetIconForFile(const std::filesystem::path& filePath)
    {
        if (std::filesystem::is_directory(filePath))
        {
            if (!std::filesystem::is_empty(filePath))
                return m_DirectoryOpenIcon;
            else
                return m_DirectoryIcon;
        }

        auto extension = filePath.extension().string();
        return m_FileTextIcon;
    }

} // namespace Titan