#include "ContentBrowserPanel.h"
#include "Titan/FontAwesome7.h"
#include "Titan/Renderer/GeometryRenderer.h"
#include "Titan/Utils/String.h"
namespace Titan
{
    extern const std::filesystem::path g_AssetPath = "assets";

    ContentBrowserPanel::ContentBrowserPanel() : m_CurrentDirectory(g_AssetPath), m_Selected("") {}

    void ContentBrowserPanel::OnImGuiRender(bool* openBrowser, bool* openFile)
    {
        RenderBrowser(openBrowser);
        RenderProperties(openFile);
    }

    void ContentBrowserPanel::RenderBrowser(bool* openBrowser)
    {
        if (openBrowser && !*openBrowser)
            return;

        ImGui::Begin(ICON_FA_FOLDER_OPEN " Content Browser", openBrowser);

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
                std::string ext = path.extension().string();
                if (ext == ".meta")
                    continue;

                auto relativePath = std::filesystem::relative(path, g_AssetPath);
                std::string filenameString = relativePath.filename().string();
                std::string filename = TruncateString(filenameString, 20);

                Ref<Texture2D> icon = Assets::GetThumbnailForFile(path);

                // --- Card layout ---
                float cardWidth = thumbnailSize + 20.0f;
                float cardHeight = thumbnailSize + 76.0f;

                // Darkened background
                ImVec4 bg = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
                float factor = 0.85f;
                bg.x *= factor;
                bg.y *= factor;
                bg.z *= factor;

                ImGui::PushID(filenameString.c_str());
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);
                ImGui::PushStyleColor(ImGuiCol_ChildBg, bg);

                // Begin card
                bool opened = ImGui::BeginChild("Card", {cardWidth, cardHeight}, false,
                                                ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

                // --- card rect for hover outline ---
                ImVec2 cardMin = ImGui::GetItemRectMin();
                ImVec2 cardMax = ImGui::GetItemRectMax();
                bool hovered = ImGui::IsItemHovered();

                if (opened)
                {
                    ImGui::BeginGroup();
                    {
                        // --- Thumbnail ---
                        float centerThumbX = (cardWidth - thumbnailSize) * 0.5f;
                        ImGui::SetCursorPosX(centerThumbX);

                        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);

                        ImVec2 imagePos = ImGui::GetCursorScreenPos();
                        ImGui::Image(icon->GetNativeTexture(), {thumbnailSize, thumbnailSize}, {0, 1}, {1, 0});

                        ImGui::SetCursorScreenPos(imagePos);
                        ImGui::InvisibleButton("##thumbnail", {thumbnailSize, thumbnailSize});

                        // Click
                        if (ImGui::IsItemClicked())
                        {
                            m_Selected = path;
                            m_SelectedType = Assets::GetTypeForFile(path);
                            m_SelectedMeta = Assets::LoadMeta(path);
                        }

                        // Double-click open
                        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                        {
                            if (directoryEntry.is_directory())
                                m_CurrentDirectory /= path.filename();
                        }

                        // Drag-drop
                        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
                        {
                            const wchar_t* itemPath = relativePath.c_str();
                            ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath,
                                                      (wcslen(itemPath) + 1) * sizeof(wchar_t));

                            ImGui::TextUnformatted(filename.c_str());
                            ImGui::EndDragDropSource();
                        }

                        // --- Filename ---
                        ImGui::Spacing();
                        float nameWidth = ImGui::CalcTextSize(filename.c_str()).x;
                        ImGui::SetCursorPosX((cardWidth - nameWidth) * 0.5f);
                        ImGui::TextWrapped("%s", filename.c_str());

                        // Add vertical spacing to avoid overlap
                        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);

                        // --- Asset Type ---
                        std::string typeStr = Assets::AssetTypeToString(Assets::GetTypeForFile(path));
                        if (directoryEntry.is_directory())
                            typeStr = "Folder";
                        if (typeStr.empty())
                            typeStr = "Unknown";

                        float typeWidth = ImGui::CalcTextSize(typeStr.c_str()).x;
                        ImGui::SetCursorPosX((cardWidth - typeWidth) * 0.5f);
                        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "%s", typeStr.c_str());
                    }
                    ImGui::EndGroup();
                }
                ImGui::EndChild();

                ImGui::PopStyleColor();
                ImGui::PopStyleVar();
                ImGui::PopID();

                ImGui::NextColumn();
            }
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            ImGui::Text("Filesystem Error: {}", e.what());
        }

        ImGui::End();
    }

    void ContentBrowserPanel::RenderProperties(bool* openFile)
    {
        if (openFile && !*openFile)
            return;

        ImGui::Begin(ICON_FA_FILE " File", openFile);
        if (!m_Selected.empty())
        {
            std::string sel = m_Selected.string();
            if (sel != m_LastSelectedStr)
            {
                if (sel.size() + 1 > m_SelectedBuf.size())
                    m_SelectedBuf.resize(sel.size() + 1);
                std::copy(sel.begin(), sel.end(), m_SelectedBuf.begin());
                m_SelectedBuf[sel.size()] = '\0';
                m_LastSelectedStr = sel;
            }

            ImGui::BeginDisabled();
            ImGui::InputText("Tag", m_SelectedBuf.data(), m_SelectedBuf.size(), ImGuiInputTextFlags_ReadOnly);
            ImGui::EndDisabled();

            auto type = m_SelectedType;
            if (type != AssetType::None)
            {
                if (type == AssetType::Texture2D)
                {
                    ImGui::Text("Type: Texture");

                    bool changed = false;

                    // Wrap options
                    const char* wrapOptions[] = {"Repeat", "MirroredRepeat", "ClampToEdge", "ClampToBorder"};
                    const char* filterOptions[] = {"Nearest", "MipmapNearest", "Linear", "MipmapLinear"};

                    std::string wrapS = "Repeat";
                    std::string wrapT = "Repeat";
                    std::string minFilter = "MipmapLinear";
                    std::string magFilter = "Linear";

                    if (m_SelectedMeta.Properties.contains("WrapS"))
                        wrapS = m_SelectedMeta.Properties["WrapS"];
                    if (m_SelectedMeta.Properties.contains("WrapT"))
                        wrapT = m_SelectedMeta.Properties["WrapT"];
                    if (m_SelectedMeta.Properties.contains("MinFilter"))
                        minFilter = m_SelectedMeta.Properties["MinFilter"];
                    if (m_SelectedMeta.Properties.contains("MagFilter"))
                        magFilter = m_SelectedMeta.Properties["MagFilter"];

                    int wrapSIdx = 0;
                    int wrapTIdx = 0;
                    int minFilterIdx = 0;
                    int magFilterIdx = 0;

                    for (int i = 0; i < IM_ARRAYSIZE(wrapOptions); i++)
                    {
                        if (wrapS == wrapOptions[i])
                            wrapSIdx = i;
                        if (wrapT == wrapOptions[i])
                            wrapTIdx = i;
                    }
                    for (int i = 0; i < IM_ARRAYSIZE(filterOptions); i++)
                    {
                        if (minFilter == filterOptions[i])
                            minFilterIdx = i;
                        if (magFilter == filterOptions[i])
                            magFilterIdx = i;
                    }

                    if (ImGui::Combo("Wrap S", &wrapSIdx, wrapOptions, IM_ARRAYSIZE(wrapOptions)))
                    {
                        m_SelectedMeta.Properties["WrapS"] = wrapOptions[wrapSIdx];
                        changed = true;
                    }

                    if (ImGui::Combo("Wrap T", &wrapTIdx, wrapOptions, IM_ARRAYSIZE(wrapOptions)))
                    {
                        m_SelectedMeta.Properties["WrapT"] = wrapOptions[wrapTIdx];
                        changed = true;
                    }

                    if (ImGui::Combo("Min Filter", &minFilterIdx, filterOptions, IM_ARRAYSIZE(filterOptions)))
                    {
                        m_SelectedMeta.Properties["MinFilter"] = filterOptions[minFilterIdx];
                        changed = true;
                    }

                    if (ImGui::Combo("Mag Filter", &magFilterIdx, filterOptions, IM_ARRAYSIZE(filterOptions)))
                    {
                        m_SelectedMeta.Properties["MagFilter"] = filterOptions[magFilterIdx];
                        changed = true;
                    }

                    if (changed)
                    {
                        // Update meta in memory and on disk, then reload the texture with new settings
                        Assets::UpdateMeta(m_Selected, m_SelectedMeta);
                        Assets::SaveMetaToDisk(m_Selected);
                        Assets::Reload<Texture2D>(m_Selected);
                        GeometryRenderer::ClearTextureCache();
                    }
                }
                else
                {
                    ImGui::Text("Type: Unsupported");
                }
            }
            else
            {
                ImGui::Text("Type: Folder");
            }
        }
        ImGui::End();
    }

} // namespace Titan