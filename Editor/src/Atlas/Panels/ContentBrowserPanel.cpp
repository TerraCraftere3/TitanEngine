#include "ContentBrowserPanel.h"

namespace Titan
{
    extern const std::filesystem::path g_AssetPath = "assets";

    ContentBrowserPanel::ContentBrowserPanel() : m_CurrentDirectory(g_AssetPath)
    {
        m_DirectoryIcon = Texture2D::Create("resources/icons/folder.svg");
        m_DirectoryOpenIcon = Texture2D::Create("resources/icons/folder-opened.svg");
        m_FileTextIcon = Texture2D::Create("resources/icons/file.svg");
        m_FileCodeIcon = Texture2D::Create("resources/icons/file-code.svg");
        m_FileImageIcon = Texture2D::Create("resources/icons/file-media.svg");
    }

    void ContentBrowserPanel::OnImGuiRender()
    {
        ImGui::Begin("Content Browser");

        try
        {
            {
                std::filesystem::path relativePath = std::filesystem::relative(m_CurrentDirectory, g_AssetPath);

                if (ImGui::InvisibleButton("##root_btn", ImVec2(0, 0)))
                {
                }
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.1f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.15f));

                if (ImGui::Button("assets"))
                    m_CurrentDirectory = g_AssetPath;

                std::filesystem::path accumulatedPath = g_AssetPath;
                for (const auto& part : relativePath)
                {
                    if (part == ".")
                        continue;

                    accumulatedPath /= part;

                    ImGui::SameLine();
                    ImGui::TextUnformatted(">");
                    ImGui::SameLine();

                    std::string partStr = part.string();
                    if (ImGui::Button(partStr.c_str()))
                        m_CurrentDirectory = accumulatedPath;
                }

                ImGui::PopStyleColor(3);
                ImGui::Separator();
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
                auto relativePath = std::filesystem::relative(path, g_AssetPath);
                std::string filenameString = relativePath.filename().string();

                Ref<Texture2D> icon = GetIconForFile(path);

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                ImGui::BeginGroup();

                // --- Image ---
                ImGui::ImageButton(filenameString.c_str(), icon->GetNativeTexture(), {thumbnailSize, thumbnailSize},
                                   {0, 1}, {1, 0});

                if (ImGui::BeginDragDropSource())
                {
                    const wchar_t* itemPath = relativePath.c_str();
                    ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath,
                                              (wcslen(itemPath) + 1) * sizeof(wchar_t));
                    ImGui::EndDragDropSource();
                }
                ImGui::PopStyleColor();

                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    if (directoryEntry.is_directory())
                        m_CurrentDirectory /= path.filename();
                }

                // --- Centered Text ---
                float textWidth = ImGui::CalcTextSize(filenameString.c_str()).x;
                float textOffset = (thumbnailSize - textWidth) * 0.5f;
                if (textOffset > 0.0f)
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textOffset);

                ImGui::TextWrapped("%s", filenameString.c_str());

                ImGui::EndGroup();
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

        std::string ext = filePath.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga")
            return m_FileImageIcon;

        if (ext == ".vert" || ext == ".frag" || ext == ".vs" || ext == ".fs" || ext == ".shader" || ext == ".glsl" ||
            ext == ".hlsl")
            return m_FileCodeIcon;

        return m_FileTextIcon;
    }

} // namespace Titan