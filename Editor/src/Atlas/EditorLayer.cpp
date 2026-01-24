#include "EditorLayer.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/LogPanel.h"
#include "Panels/ProfilerPanel.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Renderer/Thumbnails.h"

#include <Titan/Core/Application.h>
#include <Titan/Core/Input.h>
#include <Titan/FontAwesome7.h>
#include <Titan/Project/Project.h>
#include <Titan/Renderer/GeometryRenderer.h>
#include <Titan/Renderer/RenderCommand.h>
#include <Titan/Renderer/Renderer2D.h>
#include <Titan/Renderer/SceneRenderer.h>
#include <Titan/Scene/Assets.h>
#include <Titan/Scene/Components.h>
#include <Titan/Scene/SceneSerializer.h>
#include <Titan/Scripting/ScriptEngine.h>
#include <Titan/Utils/Math.h>
#include <Titan/Utils/PlatformUtils.h>
#include <Titan/Utils/String.h>
namespace Titan
{
    EditorLayer::EditorLayer() : Layer("EditorLayer") {}

    void EditorLayer::OnAttach()
    {
        // Setup
        Application::GetInstance()->GetWindow().SetVSync(false);

        std::filesystem::path configPath = std::filesystem::path(Filesystem::GetAppDataDirectory()) / "Config.yml";
        LoadEditorProperties(m_EditorProperties, configPath);

        // Initialize panels
        m_SceneHierarchyPanel = CreateScope<SceneHierarchyPanel>();
        m_ContentBrowserPanel = CreateScope<ContentBrowserPanel>();
        m_LogPanel = CreateScope<LogPanel>();
        m_ProfilerPanel = CreateScope<ProfilerPanel>();

        NewScene();
        m_SceneHierarchyPanel->SetContext(m_ActiveScene);
        m_EditorScene = m_ActiveScene;

        for (int i = 0; i < 4; ++i)
        {
            m_EditorCameras[i] = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);
            m_SceneRenderers[i] = SceneRenderer::Create();
        }
        // Give each view a slight different starting angle
        m_EditorCameras[0].MouseRotate(glm::vec2(-0.5f, 0.5f));
        m_EditorCameras[1].MouseRotate(glm::vec2(0.3f, 0.2f));
        m_EditorCameras[2].MouseRotate(glm::vec2(-0.2f, -0.3f));
        m_EditorCameras[3].MouseRotate(glm::vec2(0.1f, -0.4f));

        m_StartIcon = Assets::Load<Texture2D>("resources/icons/play.svg");
        m_StopIcon = Assets::Load<Texture2D>("resources/icons/stop.svg");
        m_SimulateIcon = Assets::Load<Texture2D>("resources/icons/simulate.svg");

        Application::GetInstance()->UpdateTheme(m_EditorProperties.Theme);

        Thumbnails::Init();

        OpenScene(Project::GetAssetDirectory() / Project::GetActive()->GetConfig().StartScene);
    }

    void EditorLayer::OnDetach()
    {
        std::filesystem::path configPath = std::filesystem::path(Filesystem::GetAppDataDirectory()) / "Config.yml";
        SaveEditorProperties(m_EditorProperties, configPath);

        if (m_SceneState == SceneState::Play)
            m_ActiveScene->OnRuntimeStop();
        else if (m_SceneState == SceneState::Simulate)
            m_ActiveScene->OnSimulationStop();

        m_ActiveScene = nullptr;

        Thumbnails::Shutdown();
    }

    void EditorLayer::OnUpdate(Timestep ts)
    {
        if (ts.GetSeconds() > 0.0f)
            m_FPS = 1.0f / ts.GetSeconds();

        RenderCommand::ResetStats();
        switch (m_SceneState)
        {
            case SceneState::Edit:
            {
                if (m_EditorProperties.EnableMultiViewports)
                {
                    for (int i = 0; i < 4; ++i)
                        m_EditorCameras[i].OnUpdate(ts);
                }
                else
                {
                    m_EditorCameras[0].OnUpdate(ts);
                }

                m_ActiveScene->OnUpdateEditor(ts, m_EditorCameras[0]);
                if (m_EnableRender)
                {
                    if (m_EditorProperties.EnableMultiViewports)
                    {
                        for (uint32_t i = 0; i < 4; ++i)
                            m_SceneRenderers[i]->RenderSceneEditor(m_ActiveScene, m_EditorCameras[i],
                                                                   m_EditorProperties.Overlays[i]);
                    }
                    else
                    {
                        m_SceneRenderers[0]->RenderSceneEditor(m_ActiveScene, m_EditorCameras[0],
                                                               m_EditorProperties.Overlays[0]);
                    }
                }
                break;
            }
            case SceneState::Simulate:
            {
                if (m_EditorProperties.EnableMultiViewports)
                {
                    for (int i = 0; i < 4; ++i)
                        m_EditorCameras[i].OnUpdate(ts);
                }
                else
                {
                    m_EditorCameras[0].OnUpdate(ts);
                }

                m_ActiveScene->OnUpdateSimulation(ts, m_EditorCameras[0]);
                if (m_EnableRender)
                {
                    if (m_EditorProperties.EnableMultiViewports)
                    {
                        for (uint32_t i = 0; i < 4; ++i)
                            m_SceneRenderers[i]->RenderSceneEditor(m_ActiveScene, m_EditorCameras[i],
                                                                   m_EditorProperties.Overlays[i]);
                    }
                    else
                    {
                        m_SceneRenderers[0]->RenderSceneEditor(m_ActiveScene, m_EditorCameras[0],
                                                               m_EditorProperties.Overlays[0]);
                    }
                }
                break;
            }
            case SceneState::Play:
            {
                m_ActiveScene->OnUpdateRuntime(ts);
                if (m_EnableRender)
                    m_SceneRenderers[0]->RenderSceneRuntime(m_ActiveScene);
                break;
            }
        }

        if (m_EditorProperties.EnableMultiViewports)
        {
            for (int i = 0; i < 4; ++i)
                m_EditorCameras[i].SetBlockEvents(!m_SubViewportHovered[i]);
        }
        else
        {
            m_EditorCameras[0].SetBlockEvents(!m_SubViewportHovered[0]);
        }
    }

    void EditorLayer::OnImGuiRender(ImGuiContext* ctx)
    {
        ImGui::SetCurrentContext(ctx);

        RenderDockspace();

        if (m_EditorProperties.ShowSceneHierarchy)
            m_SceneHierarchyPanel->OnImGuiRender(&m_EditorProperties.ShowSceneHierarchy,
                                                 &m_EditorProperties.ShowSceneHierarchyProperties);
        if (m_EditorProperties.ShowContentBrowser)
            m_ContentBrowserPanel->OnImGuiRender(&m_EditorProperties.ShowContentBrowser,
                                                 &m_EditorProperties.ShowContentBrowserFile);

        RenderStatisticsPanel(&m_EditorProperties.ShowStatistics);
        RenderViewport(&m_EditorProperties.ShowViewport);

        if (m_EditorProperties.ShowLog)
            m_LogPanel->OnImGuiRender(&m_EditorProperties.ShowLog);

        if (m_EditorProperties.ShowProfiler)
            m_ProfilerPanel->OnImGuiRender(&m_EditorProperties.ShowProfiler);
    }

    void EditorLayer::OnEvent(Event& event)
    {
        if (m_EditorProperties.EnableMultiViewports)
        {
            for (int i = 0; i < 4; ++i)
                m_EditorCameras[i].OnEvent(event);
        }
        else
        {
            m_EditorCameras[0].OnEvent(event);
        }

        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<KeyPressedEvent>(TI_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
        dispatcher.Dispatch<MouseButtonPressedEvent>(TI_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
    }

    // ============================================================================
    // Private Helper Methods
    // ============================================================================

    void EditorLayer::RenderDockspace()
    {
        static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
                                       ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                       ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                       ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        ImGui::Begin("DockSpace", nullptr, windowFlags);
        ImGui::PopStyleVar(3);

        ImGuiID dockspaceID = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockspaceFlags);

        RenderMenuBar();

        ImGui::End();
    }

    void EditorLayer::RenderMenuBar()
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New", "Ctrl+N"))
                    NewScene();

                if (ImGui::MenuItem("Open", "Ctrl+O"))
                    OpenScene();

                if (ImGui::MenuItem("Save As", "Ctrl+Shift+S"))
                    SaveSceneAs();

                ImGui::Separator();

                if (ImGui::MenuItem("Exit", "Alt+F4"))
                    Application::GetInstance()->Close();

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Scripts"))
            {
                if (ImGui::MenuItem("Reload", "Ctrl+R"))
                    ScriptEngine::ReloadAssembly();

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Windows"))
            {
                if (ImGui::MenuItem("Scene Hierarchy", nullptr, &m_EditorProperties.ShowSceneHierarchy))
                {
                }
                if (ImGui::MenuItem("Properties", nullptr, &m_EditorProperties.ShowSceneHierarchyProperties))
                {
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Content Browser", nullptr, &m_EditorProperties.ShowContentBrowser))
                {
                }
                if (ImGui::MenuItem("File/Properties", nullptr, &m_EditorProperties.ShowContentBrowserFile))
                {
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Statistics", nullptr, &m_EditorProperties.ShowStatistics))
                {
                }
                if (ImGui::MenuItem("Viewport", nullptr, &m_EditorProperties.ShowViewport))
                {
                }
                if (ImGui::MenuItem("Profiler", nullptr, &m_EditorProperties.ShowProfiler))
                {
                }
                if (ImGui::MenuItem("Log", nullptr, &m_EditorProperties.ShowLog))
                {
                }

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Styles"))
            {
                if (ImGui::MenuItem("Dark", nullptr, m_EditorProperties.Theme == UITheme::Dark))
                {
                    m_EditorProperties.Theme = UITheme::Dark;
                    Application::GetInstance()->UpdateTheme(m_EditorProperties.Theme);
                }
                if (ImGui::MenuItem("OLED", nullptr, m_EditorProperties.Theme == UITheme::OLED))
                {
                    m_EditorProperties.Theme = UITheme::OLED;
                    Application::GetInstance()->UpdateTheme(m_EditorProperties.Theme);
                }

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Project"))
            {
                ImGui::Text("Path: %s", std::filesystem::absolute(Project::GetProjectDirectory()).string().c_str());
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
    }

    void EditorLayer::RenderStatisticsPanel(bool* open)
    {
        ImGui::Begin("Statistics", open);

        ImGui::Text("FPS: %.1f", m_FPS);
        ImGui::Text("Draw Calls: %d", RenderCommand::GetStats().GetTotalDrawCalls());

        ImGui::SeparatorText("Backend");
        auto& settings = RenderCommand::GetBackend();
        ImGui::Text("API: %s", settings.Renderer.c_str());
        ImGui::Text("Version: %s", settings.Version.c_str());
        ImGui::Text("Vendor: %s", settings.Vendor.c_str());

        ImGui::Separator();
        ImGui::SeparatorText("Features");
        ImGui::BeginDisabled();
        ImGui::Checkbox("Compute Shaders", &const_cast<bool&>(settings.SupportsComputeShaders));
        ImGui::Checkbox("Tessellation Shaders", &const_cast<bool&>(settings.SupportsTessellationShaders));
        ImGui::Checkbox("Geometry Shaders", &const_cast<bool&>(settings.SupportsGeometryShaders));
        ImGui::Checkbox("Raytracing", &const_cast<bool&>(settings.SupportsRaytracing));
        ImGui::EndDisabled();

        ImGui::Separator();
        ImGui::SeparatorText("Limits");
        ImGui::BeginDisabled();
        ImGui::InputScalar("Max Texture Units", ImGuiDataType_U32, &const_cast<uint32_t&>(settings.MaxTextureUnits),
                           nullptr, nullptr);
        ImGui::InputScalar("Max Texture Size", ImGuiDataType_U32, &const_cast<uint32_t&>(settings.MaxTextureSize),
                           nullptr, nullptr);
        ImGui::EndDisabled();

        ImGui::End();
    }

    void EditorLayer::RenderViewport(bool* open)
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        ImGui::SetNextWindowSizeConstraints(ImVec2(256, 256), ImVec2(8192, 8192));
        ImGui::Begin(ICON_FA_CIRCLE_INFO " Viewport###Viewport", open,
                     ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        // Render the scene control toolbar at the top
        RenderSceneControlToolbar();

        UpdateViewportBounds();
        HandleViewportResize();

        if (m_SceneState == SceneState::Edit)
            RenderGizmoToolbar();

        RenderViewportImage();
        HandleGizmoManipulation();

        ImGui::End();
        ImGui::PopStyleVar(2);
    }

    void EditorLayer::RenderSceneControlToolbar()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 0));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

        ImGui::BeginChild("##toolbar", ImVec2(0, 40), false,
                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);

        bool isRunning = (m_SceneState == SceneState::Play || m_SceneState == SceneState::Simulate);

        if (isRunning)
        {
            float buttonSize = 32.0f;
            float totalWidth = buttonSize * 1; // One Button
            float offsetX = (ImGui::GetContentRegionAvail().x - totalWidth) * 0.5f;
            ImGui::SetCursorPosX(offsetX);

            // ----------------- STOP BUTTON -----------------
            if (ImGui::ImageButton("ScenePlayButton", m_StopIcon->GetNativeTexture(), ImVec2(buttonSize, buttonSize),
                                   ImVec2(0, 0), ImVec2(1, 1)))
                OnSceneStop();
        }
        else
        {
            float buttonSize = 32.0f;
            float totalWidth = buttonSize * 2 + 8; // Two buttons + spacing
            float offsetX = (ImGui::GetContentRegionAvail().x - totalWidth) * 0.5f;
            ImGui::SetCursorPosX(offsetX);

            // ----------------- PLAY BUTTON -----------------
            {
                if (ImGui::ImageButton("ScenePlayButton", m_StartIcon->GetNativeTexture(),
                                       ImVec2(buttonSize, buttonSize), ImVec2(0, 0), ImVec2(1, 1)))
                {
                    OnScenePlay();
                }
            }

            ImGui::SameLine(0, 8);

            // ----------------- SIMULATE BUTTON -----------------
            {
                if (ImGui::ImageButton("SceneSimulateButton", m_SimulateIcon->GetNativeTexture(),
                                       ImVec2(buttonSize, buttonSize), ImVec2(0, 0), ImVec2(1, 1)))
                {
                    OnSceneSimulate();
                }
            }
        }

        ImGui::EndChild();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar(2);

        ImGui::Separator();
    }

    void EditorLayer::UpdateViewportBounds()
    {
        auto viewportOffset = ImGui::GetCursorPos();
        ImVec2 minBound = ImGui::GetWindowPos();
        minBound.x += viewportOffset.x;
        minBound.y += viewportOffset.y;

        ImVec2 maxBound = {minBound.x + m_ViewportSize.x, minBound.y + m_ViewportSize.y};
        m_ViewportBounds[0] = {minBound.x, minBound.y};
        m_ViewportBounds[1] = {maxBound.x, maxBound.y};

        m_ViewportFocused = ImGui::IsWindowFocused();
        m_ViewportHovered = ImGui::IsWindowHovered();
    }

    void EditorLayer::HandleViewportResize()
    {
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        glm::vec2 newSize = {viewportPanelSize.x, viewportPanelSize.y};

        static bool s_PrevMultiViewportState = m_EditorProperties.EnableMultiViewports;
        bool viewportModeChanged = (s_PrevMultiViewportState != m_EditorProperties.EnableMultiViewports);
        s_PrevMultiViewportState = m_EditorProperties.EnableMultiViewports;

        if (m_ViewportSize != newSize || viewportModeChanged)
        {
            m_ViewportSize = newSize;

            if (m_EditorProperties.EnableMultiViewports)
            {
                // Split into 2x2 cells
                uint32_t cellW = (uint32_t)(newSize.x * 0.5f);
                uint32_t cellH = (uint32_t)(newSize.y * 0.5f);

                for (uint32_t i = 0; i < 4; ++i)
                {
                    m_SceneRenderers[i]->Resize(cellW, cellH);
                    m_EditorCameras[i].SetViewportSize((float)cellW, (float)cellH);
                }
            }
            else
            {
                // Single viewport - use full size for view 0
                m_SceneRenderers[0]->Resize((uint32_t)newSize.x, (uint32_t)newSize.y);
                m_EditorCameras[0].SetViewportSize(newSize.x, newSize.y);
            }

            // Keep scene aware of the main viewport size (use full region)
            m_ActiveScene->OnViewportResize(static_cast<uint32_t>(newSize.x), static_cast<uint32_t>(newSize.y));
        }
    }

    void EditorLayer::RenderGizmoToolbar()
    {
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 contentPos = ImGui::GetWindowContentRegionMin();
        ImVec2 toolbarPos = {windowPos.x + contentPos.x + 10.0f, windowPos.y + contentPos.y + 10.0f};

        ImGui::SetNextWindowPos(toolbarPos, ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.75f);

        ImGuiWindowFlags toolbarFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar |
                                        ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse;

        ImGui::Begin("GizmoToolbar", nullptr, toolbarFlags);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 16));

        constexpr float buttonSize = 25.0f;

        if (ImGui::Button("T", ImVec2(buttonSize, buttonSize)))
            m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
        ImGui::SameLine();

        if (ImGui::Button("R", ImVec2(buttonSize, buttonSize)))
            m_GizmoType = ImGuizmo::OPERATION::ROTATE;
        ImGui::SameLine();

        if (ImGui::Button("S", ImVec2(buttonSize, buttonSize)))
            m_GizmoType = ImGuizmo::OPERATION::SCALE;

        ImGui::SameLine();
        ImGui::Checkbox("Enable Multi Viewports", &m_EditorProperties.EnableMultiViewports);
        ImGui::SameLine();
        ImGui::Checkbox("Overlay", &m_EditorProperties.Overlays[0].enableOverlay);
        ImGui::SameLine();
        ImGui::Checkbox("AABB", &m_EditorProperties.Overlays[0].enableBoundingBoxRender);
        ImGui::SameLine();
        ImGui::Checkbox("Wireframe", &m_EditorProperties.Overlays[0].enableWireframe);

        ImGui::PopStyleVar(2);
        ImGui::End();
    }

    void EditorLayer::RenderViewportImage()
    {
        // Overall viewport rect
        m_ViewportImagePos = ImGui::GetCursorScreenPos();
        m_ViewportImageSize = ImGui::GetContentRegionAvail();

        if (m_EditorProperties.EnableMultiViewports)
        {
            // Split into 2x2 grid
            float cellW = m_ViewportImageSize.x * 0.5f;
            float cellH = m_ViewportImageSize.y * 0.5f;

            for (int row = 0; row < 2; ++row)
            {
                for (int col = 0; col < 2; ++col)
                {
                    int idx = row * 2 + col;
                    ImVec2 pos = {m_ViewportImagePos.x + col * cellW, m_ViewportImagePos.y + row * cellH};
                    ImVec2 size = {cellW, cellH};

                    m_SubViewportImagePos[idx] = pos;
                    m_SubViewportImageSize[idx] = size;

                    ImGui::SetCursorScreenPos(pos);
                    auto fb = m_SceneRenderers[idx]->GetFramebuffer();
                    if (fb)
                    {
                        ImGui::Image(fb->GetColorAttachmentTexture(0)->GetNativeTexture(), size, ImVec2(0, 1),
                                     ImVec2(1, 0));
                    }
                    else
                    {
                        ImGui::Dummy(size);
                    }

                    HandleSceneDragDrop();

                    // Hover detection and active selection
                    m_SubViewportHovered[idx] = ImGui::IsItemHovered();
                    if (ImGui::IsItemClicked())
                        m_ActiveViewportIndex = idx;

                    // Draw a small overlay with flags on the active viewport
                    if (idx == m_ActiveViewportIndex)
                    {
                        ImDrawList* dl = ImGui::GetWindowDrawList();
                        ImVec2 boxPos = {pos.x + 8, pos.y + 8};
                        ImGui::SetCursorScreenPos({boxPos.x + 6, boxPos.y + 3});
                        ImGui::BeginGroup();
                        ImGui::Checkbox("Overlay", &m_EditorProperties.Overlays[idx].enableOverlay);
                        ImGui::SameLine();
                        ImGui::Checkbox("AABB", &m_EditorProperties.Overlays[idx].enableBoundingBoxRender);
                        ImGui::SameLine();
                        ImGui::Checkbox("Wireframe", &m_EditorProperties.Overlays[idx].enableWireframe);
                        ImGui::EndGroup();
                    }
                }
            }
        }
        else
        {
            // Single viewport mode - only render view 0
            m_SubViewportImagePos[0] = m_ViewportImagePos;
            m_SubViewportImageSize[0] = m_ViewportImageSize;

            auto fb = m_SceneRenderers[0]->GetFramebuffer();
            if (fb)
            {
                ImGui::Image(fb->GetColorAttachmentTexture(0)->GetNativeTexture(), m_ViewportImageSize, ImVec2(0, 1),
                             ImVec2(1, 0));
                HandleSceneDragDrop();
            }

            m_SubViewportHovered[0] = ImGui::IsItemHovered();
            m_ActiveViewportIndex = 0;

            // Reset hover for other viewports
            for (int i = 1; i < 4; ++i)
                m_SubViewportHovered[i] = false;
        }
    }

    void EditorLayer::HandleSceneDragDrop()
    {
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
            {
                const wchar_t* path = static_cast<const wchar_t*>(payload->Data);
                OpenScene(Project::GetAssetDirectory() / path);
            }
            ImGui::EndDragDropTarget();
        }
    }

    void EditorLayer::HandleGizmoManipulation()
    {
        if (m_SceneState == SceneState::Play)
            return;

        Entity selected = m_SceneHierarchyPanel->GetSelectedEntity();

        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();

        int activeIdx = m_ActiveViewportIndex;
        ImVec2 gizmoPos = m_SubViewportImagePos[activeIdx];
        ImVec2 gizmoSize = m_SubViewportImageSize[activeIdx];
        ImGuizmo::SetRect(gizmoPos.x, gizmoPos.y, gizmoSize.x, gizmoSize.y);

        const glm::mat4& proj = m_EditorCameras[activeIdx].GetProjection();
        glm::mat4 view = m_EditorCameras[activeIdx].GetViewMatrix();

        bool snap = Input::IsKeyPressed(Key::LeftControl);
        float snapValue = (m_GizmoType == ImGuizmo::OPERATION::ROTATE) ? 45.0f : 0.5f;
        float snapValues[3] = {snapValue, snapValue, snapValue};

        if (selected && selected.HasComponent<TransformComponent>() && m_GizmoType != -1)
        {
            auto& tc = selected.GetComponent<TransformComponent>();
            glm::mat4 transform = tc.GetTransform();

            ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(proj),
                                 static_cast<ImGuizmo::OPERATION>(m_GizmoType), ImGuizmo::LOCAL,
                                 glm::value_ptr(transform), nullptr, snap ? snapValues : nullptr);

            if (ImGuizmo::IsUsing())
            {
                glm::vec3 t, r, s;
                Math::DecomposeTransform(transform, t, r, s);

                glm::vec3 deltaRot = r - tc.Rotation;
                tc.Translation = t;
                tc.Rotation += deltaRot;
                tc.Scale = s;
            }
        }
    }

    // ============================================================================
    // Event Handlers
    // ============================================================================

    bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
    {
        if (e.GetRepeatCount() > 0)
            return false;

        bool control = Input::IsKeyPressed(KeyCode::LeftControl) || Input::IsKeyPressed(KeyCode::RightControl);
        bool shift = Input::IsKeyPressed(KeyCode::LeftShift) || Input::IsKeyPressed(KeyCode::RightShift);

        switch (e.GetKeyCode())
        {
            case KeyCode::N:
            {
                if (control)
                    NewScene();
                break;
            }
            case KeyCode::O:
            {
                if (control)
                    OpenScene();
                break;
            }
            case Key::S:
            {
                if (control)
                {
                    if (shift)
                        SaveSceneAs();
                    else
                        SaveScene();
                }

                break;
            }
            case Key::D:
            {
                if (control)
                    OnDuplicateEntity();

                break;
            }
            case KeyCode::Q:
                if (!ImGuizmo::IsUsing())
                {
                    m_GizmoType = -1;
                }
                break;

            case KeyCode::W:
                if (!ImGuizmo::IsUsing())
                {
                    m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
                }
                break;

            case KeyCode::E:
                if (!ImGuizmo::IsUsing())
                {
                    m_GizmoType = ImGuizmo::OPERATION::ROTATE;
                }
                break;

            case KeyCode::R:
                if (control)
                {
                    ScriptEngine::ReloadAssembly();
                }
                else if (!ImGuizmo::IsUsing())
                {
                    m_GizmoType = ImGuizmo::OPERATION::SCALE;
                }
                break;
        }

        return false;
    }

    bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
    {
        if (e.GetMouseButton() == static_cast<int>(MouseButton::ButtonLeft))
        {
            // Determine which viewport sub-panel was clicked
            ImVec2 mouse = ImGui::GetMousePos();
            int clickedViewIdx = -1;
            for (int i = 0; i < 4; ++i)
            {
                if (m_SubViewportHovered[i])
                {
                    clickedViewIdx = i;
                    break;
                }
            }

            if (clickedViewIdx >= 0 && !ImGuizmo::IsOver() && !Input::IsKeyPressed(Key::LeftAlt))
            {
                float mx = mouse.x - m_SubViewportImagePos[clickedViewIdx].x;
                float my = mouse.y - m_SubViewportImagePos[clickedViewIdx].y;
                int mouseX = static_cast<int>(mx);
                int mouseY = static_cast<int>(my);

                if (mouseX >= 0 && mouseY >= 0 && mouseX < static_cast<int>(m_SubViewportImageSize[clickedViewIdx].x) &&
                    mouseY < static_cast<int>(m_SubViewportImageSize[clickedViewIdx].y))
                {
                    int pixel = m_SceneRenderers[clickedViewIdx]->GetFramebuffer()->ReadPixel(1, mouseX, mouseY);
                    auto entity = Entity(static_cast<entt::entity>(pixel), m_ActiveScene.get());
                    m_SceneHierarchyPanel->SetSelectedEntity(entity);
                }
            }
        }
        return false;
    }

    // ============================================================================
    // Scene Management
    // ============================================================================

    void EditorLayer::NewScene()
    {
        m_ActiveScene = CreateRef<Scene>();
        m_ActiveScene->OnViewportResize(static_cast<uint32_t>(m_ViewportSize.x),
                                        static_cast<uint32_t>(m_ViewportSize.y));

        {
            auto cube = m_ActiveScene->CreateEntity("Cube");
            auto& mrc = cube.AddComponent<MeshRendererComponent>();
            mrc.MeshRef = Mesh::Create("cube");
        }

        {
            auto camera = m_ActiveScene->CreateEntity("Camera");
            auto& t = camera.GetComponent<TransformComponent>();
            t.Translation = glm::vec3(-5.0f, 3.0f, 5.0f);
            auto& cc = camera.AddComponent<CameraComponent>();
            cc.Camera.SetProjectionType(SceneCamera::ProjectionType::Perspective);
            auto& lcc = camera.AddComponent<LookAtComponent>();
        }

        {
            auto skybox = m_ActiveScene->CreateEntity("Skybox");
            auto& sbc = skybox.AddComponent<SkyboxComponent>();
            sbc.mode = SkyboxComponent::Mode::Colorramp;
        }

        {
            auto pp = m_ActiveScene->CreateEntity("Post Processing");
            auto ppc = pp.AddComponent<PostFXComponent>();
        }

        {
            auto light = m_ActiveScene->CreateEntity("Directional Light");
            auto& lc = light.AddComponent<DirectionalLightComponent>();
            lc.Direction = glm::vec3(-0.5f, -1.0f, -0.5f);
        }

        m_SceneHierarchyPanel->SetContext(m_ActiveScene);

        m_EditorScene = m_ActiveScene;
        m_EditorScenePath = std::filesystem::path();

        GeometryRenderer::ClearCache();
    }

    void EditorLayer::OpenScene(const std::filesystem::path& path)
    {
        if (m_SceneState != SceneState::Edit)
            OnSceneStop();

        if (path.extension().string() != ".titan")
        {
            TI_WARN("Could not load {0} - not a scene file", path.filename().string());
            return;
        }

        if (m_SceneState == SceneState::Play)
            OnSceneStop();

        Assets::Unload(m_EditorScenePath);

        Ref<Scene> newScene = Assets::Load<Scene>(path);
        m_EditorScene = newScene;
        m_EditorScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        m_SceneHierarchyPanel->SetContext(m_EditorScene);

        m_ActiveScene = m_EditorScene;
        m_EditorScenePath = path;

        GeometryRenderer::ClearCache();
    }

    void EditorLayer::SaveScene()
    {
        if (!m_EditorScenePath.empty())
            SerializeScene(m_ActiveScene, m_EditorScenePath);
        else
            SaveSceneAs();
    }

    void EditorLayer::OpenScene()
    {
        std::string filepath = FileDialogs::OpenFile("Titan Scene (*.titan)\0*.titan\0");
        if (!filepath.empty())
        {
            OpenScene(filepath);
        }
    }

    void EditorLayer::SaveSceneAs()
    {
        std::string filepath = FileDialogs::SaveFile("Titan Scene (*.titan)\0*.titan\0");
        if (!filepath.empty())
        {
            SerializeScene(m_ActiveScene, filepath);
            m_EditorScenePath = filepath;
        }
    }

    void EditorLayer::SerializeScene(Ref<Scene> scene, const std::filesystem::path& path)
    {
        SceneSerializer serializer(scene);
        serializer.Serialize(path.string());
    }

    void EditorLayer::OnScenePlay()
    {
        if (m_SceneState == SceneState::Simulate)
            OnSceneStop();

        m_SceneState = SceneState::Play;

        m_ActiveScene = Scene::Copy(m_EditorScene);
        m_ActiveScene->OnRuntimeStart();

        m_SceneHierarchyPanel->SetContext(m_ActiveScene);
    }

    void EditorLayer::OnSceneSimulate()
    {
        if (m_SceneState == SceneState::Play)
            OnSceneStop();

        m_SceneState = SceneState::Simulate;

        m_ActiveScene = Scene::Copy(m_EditorScene);
        m_ActiveScene->OnSimulationStart();

        m_SceneHierarchyPanel->SetContext(m_ActiveScene);
    }

    void EditorLayer::OnSceneStop()
    {
        TI_ASSERT(m_SceneState == SceneState::Play || m_SceneState == SceneState::Simulate);

        if (m_SceneState == SceneState::Play)
            m_ActiveScene->OnRuntimeStop();
        else if (m_SceneState == SceneState::Simulate)
            m_ActiveScene->OnSimulationStop();

        m_SceneState = SceneState::Edit;
        m_ActiveScene = m_EditorScene;

        m_SceneHierarchyPanel->SetContext(m_ActiveScene);
    }

    void EditorLayer::OnDuplicateEntity()
    {
        if (m_SceneState != SceneState::Edit)
            return;

        Entity selectedEntity = m_SceneHierarchyPanel->GetSelectedEntity();
        if (selectedEntity)
            m_EditorScene->DuplicateEntity(selectedEntity);
    }

} // namespace Titan