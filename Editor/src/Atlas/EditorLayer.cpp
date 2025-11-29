#include "EditorLayer.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/LogPanel.h"
#include "Panels/SceneHierarchyPanel.h"

#include <Titan/Core/Application.h>
#include <Titan/Core/Input.h>
#include <Titan/FontAwesome7.h>
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
    extern const std::filesystem::path g_AssetPath;

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

        m_ActiveScene = Assets::Load<Scene>("assets/scenes/Helmet.titan");
        m_SceneHierarchyPanel->SetContext(m_ActiveScene);
        m_EditorScene = m_ActiveScene;

        m_EditorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);
        m_EditorCamera.MouseRotate(glm::vec2(-0.5f, 0.5f)); // Rotate

        m_StartIcon = Assets::Load<Texture2D>("resources/icons/play.svg");
        m_StopIcon = Assets::Load<Texture2D>("resources/icons/stop.svg");
        m_SimulateIcon = Assets::Load<Texture2D>("resources/icons/simulate.svg");
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
    }

    void EditorLayer::OnUpdate(Timestep ts)
    {
        if (ts.GetSeconds() > 0.0f)
            m_FPS = 1.0f / ts.GetSeconds();

        Renderer2D::ResetStats();
        GeometryRenderer::ResetStats();
        switch (m_SceneState)
        {
            case SceneState::Edit:
            {
                m_EditorCamera.OnUpdate(ts);

                m_ActiveScene->OnUpdateEditor(ts, m_EditorCamera);
                if (m_EnableRender)
                    SceneRenderer::RenderSceneEditor(m_ActiveScene, m_EditorCamera, m_OverlaySettings);
                break;
            }
            case SceneState::Simulate:
            {
                m_EditorCamera.OnUpdate(ts);

                m_ActiveScene->OnUpdateSimulation(ts, m_EditorCamera);
                if (m_EnableRender)
                    SceneRenderer::RenderSceneEditor(m_ActiveScene, m_EditorCamera, m_OverlaySettings);
                break;
            }
            case SceneState::Play:
            {
                m_ActiveScene->OnUpdateRuntime(ts);
                if (m_EnableRender)
                    SceneRenderer::RenderSceneRuntime(m_ActiveScene);
                break;
            }
        }

        m_EditorCamera.SetBlockEvents(!m_ViewportHovered);
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
            Profiler::Get().DrawProfilerUI();
    }

    void EditorLayer::OnEvent(Event& event)
    {
        m_EditorCamera.OnEvent(event);

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
            ImGui::EndMenuBar();
        }
    }

    void EditorLayer::RenderStatisticsPanel(bool* open)
    {
        ImGui::Begin("Statistics", open);

        ImGui::Text("FPS: %.1f", m_FPS);
        ImGui::Separator();

        auto stats2d = Renderer2D::GetStats();
        auto stats3d = GeometryRenderer::GetStats();
        ImGui::Text("Draw Calls: %d", stats2d.GetTotalDrawCalls() + stats3d.GetTotalDrawCalls());
        ImGui::Text("Meshes Rendered: %d", stats3d.GetTotalMeshCount());
        ImGui::Text("Vertices Rendered: %s",
                    FormatNumber(stats2d.GetTotalVertexCount() + stats3d.GetTotalVertexCount()).c_str());

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
        HandleSceneDragDrop();
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

        if (m_ViewportSize != newSize)
        {
            SceneRenderer::Resize(static_cast<uint32_t>(newSize.x), static_cast<uint32_t>(newSize.y));
            m_ViewportSize = newSize;
            m_ActiveScene->OnViewportResize(static_cast<uint32_t>(newSize.x), static_cast<uint32_t>(newSize.y));
            m_EditorCamera.SetViewportSize(newSize.x, newSize.y);
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
        ImGui::Checkbox("Enable AABB Rendering", &m_OverlaySettings.enableBoundingBoxRender);

        ImGui::SameLine();
        ImGui::Checkbox("Wireframe Mode", &m_OverlaySettings.enableWireframe);

        ImGui::PopStyleVar(2);
        ImGui::End();
    }

    void EditorLayer::RenderViewportImage()
    {
        m_ViewportImagePos = ImGui::GetCursorScreenPos();
        m_ViewportImageSize = ImGui::GetContentRegionAvail();

        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        ImGui::Image(SceneRenderer::GetFramebuffer()->GetColorAttachmentTexture(0)->GetNativeTexture(), viewportSize,
                     ImVec2(0, 1), ImVec2(1, 0));
    }

    void EditorLayer::HandleSceneDragDrop()
    {
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
            {
                const wchar_t* path = static_cast<const wchar_t*>(payload->Data);
                OpenScene(std::filesystem::path(g_AssetPath) / path);
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

        ImGuizmo::SetRect(m_ViewportImagePos.x, m_ViewportImagePos.y, m_ViewportImageSize.x, m_ViewportImageSize.y);

        const glm::mat4& proj = m_EditorCamera.GetProjection();
        glm::mat4 view = m_EditorCamera.GetViewMatrix();

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
            if (m_ViewportHovered && m_ViewportImageSize.x > 0 && m_ViewportImageSize.y > 0 && !ImGuizmo::IsOver() &&
                !Input::IsKeyPressed(Key::LeftAlt))
            {
                ImVec2 mouse = ImGui::GetMousePos();
                float mx = mouse.x - m_ViewportImagePos.x;
                float my = mouse.y - m_ViewportImagePos.y;
                int mouseX = static_cast<int>(mx);
                int mouseY = static_cast<int>(my);
                if (mouseX >= 0 && mouseY >= 0 && mouseX < static_cast<int>(m_ViewportImageSize.x) &&
                    mouseY < static_cast<int>(m_ViewportImageSize.y))
                {
                    int pixel = SceneRenderer::GetFramebuffer()->ReadPixel(1, mouseX, mouseY);
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
            auto skybox = m_ActiveScene->CreateEntity("Skybox");
            auto& sbc = skybox.AddComponent<SkyboxComponent>();
            sbc.mode = SkyboxComponent::Mode::Colorramp;
        }

        {
            auto camera = m_ActiveScene->CreateEntity("Camera");
            auto& t = camera.GetComponent<TransformComponent>();
            t.Translation = glm::vec3(-5.0f, 3.0f, 5.0f);
            auto& cc = camera.AddComponent<CameraComponent>();
            cc.Camera.SetProjectionType(SceneCamera::ProjectionType::Perspective);
            auto& lcc = camera.AddComponent<LookAtComponent>();
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