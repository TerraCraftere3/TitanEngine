#include "EditorLayer.h"
#include <algorithm>
#include <glm/gtx/matrix_decompose.hpp>
#include <random>

namespace Titan
{
    extern const std::filesystem::path g_AssetPath;

    EditorLayer::EditorLayer() : Layer("EditorLayer Test") {}

    void EditorLayer::OnAttach()
    {
        // Setup
        Application::GetInstance()->GetWindow().SetVSync(false);

        FramebufferSpecification fbSpec;
        fbSpec.Attachments = {FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER,
                              FramebufferTextureFormat::Depth};
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        fbSpec.Samples = 4;
        m_Framebuffer = Framebuffer::Create(fbSpec);

        m_ActiveScene = CreateRef<Scene>();
        SceneSerializer(m_ActiveScene).Deserialize("assets/scenes/Cube.titan");
        m_EditorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);

        m_SceneHierarchyPanel.SetContext(m_ActiveScene);

        m_StartIcon = Texture2D::Create("resources/icons/play.svg");
        m_StopIcon = Texture2D::Create("resources/icons/stop.svg");
    }

    void EditorLayer::OnDetach() {}

    void EditorLayer::OnUpdate(Timestep ts)
    {
        if (ts.GetSeconds() > 0.0f)
            m_FPS = 1.0f / ts.GetSeconds();

        m_Framebuffer->Bind();
        RenderCommand::SetClearColor({173.0f / 255.0f, 216.0f / 255.0f, 230.0f / 255.0f, 1.0f});
        RenderCommand::Clear();

        Renderer2D::ResetStats();
        m_Framebuffer->ClearAttachment(1, -1);
        switch (m_SceneState)
        {
            case SceneState::Edit:
            {
                m_EditorCamera.OnUpdate(ts);

                m_ActiveScene->OnUpdateEditor(ts, m_EditorCamera);
                break;
            }
            case SceneState::Play:
            {
                m_ActiveScene->OnUpdateRuntime(ts);
                break;
            }
        }

        UpdateHoveredEntity();
        m_Framebuffer->Unbind();
    }

    void EditorLayer::OnImGuiRender(ImGuiContext* ctx)
    {
        ImGui::SetCurrentContext(ctx);

        RenderDockspace();

        m_SceneHierarchyPanel.OnImGuiRender();
        m_ContentBrowserPanel.OnImGuiRender();

        RenderStatisticsPanel();
        RenderViewport();
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

    void EditorLayer::UpdateHoveredEntity()
    {
        auto [mx, my] = ImGui::GetMousePos();
        mx -= m_ViewportBounds[0].x;
        my -= m_ViewportBounds[0].y;

        glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
        my = viewportSize.y - my;

        int mouseX = static_cast<int>(mx);
        int mouseY = static_cast<int>(my);

        if (mouseX >= 0 && mouseY >= 0 && mouseX < static_cast<int>(viewportSize.x) &&
            mouseY < static_cast<int>(viewportSize.y))
        {
            int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
            m_HoveredEntity = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, m_ActiveScene.get());
        }
    }

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

                if (ImGui::MenuItem("Open...", "Ctrl+O"))
                    OpenScene();

                if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
                    SaveSceneAs();

                ImGui::Separator();

                if (ImGui::MenuItem("Exit", "Alt+F4"))
                    Application::GetInstance()->Close();

                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
    }

    void EditorLayer::RenderStatisticsPanel()
    {
        ImGui::Begin("Statistics");

        std::string hoveredName = m_HoveredEntity ? "Entity" : "None";
        ImGui::Text("Hovered Entity: %s", hoveredName.c_str());

        ImGui::Separator();

        auto stats = Renderer2D::GetStats();
        ImGui::Text("Draw Calls: %d", stats.GetTotalDrawCalls());
        ImGui::Text("Quads: %d", stats.GetTotalQuadCount());
        ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
        ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
        ImGui::Text("Triangles: %d", stats.GetTotalTriangleCount());

        ImGui::Separator();
        ImGui::Text("FPS: %.1f", m_FPS);

        ImGui::End();
    }

    void EditorLayer::RenderViewport()
    {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        ImGui::SetNextWindowSizeConstraints(ImVec2(256, 256), ImVec2(8192, 8192));
        const char* windowLabel = m_SceneState == SceneState::Edit ? "Viewport" : "Game";
        ImGui::Begin((std::string(windowLabel) + "##Viewport").c_str(), nullptr,
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

        // Center the toolbar buttons
        float buttonSize = 32.0f;
        float totalWidth = buttonSize; // Just one button for now
        float offsetX = (ImGui::GetContentRegionAvail().x - totalWidth) * 0.5f;
        ImGui::SetCursorPosX(offsetX);

        // Display play or stop button based on scene state
        Ref<Texture2D> icon = m_SceneState == SceneState::Edit ? m_StartIcon : m_StopIcon;

        if (ImGui::ImageButton("SceneStateBTN", icon->GetNativeTexture(), ImVec2(buttonSize, buttonSize), ImVec2(0, 0),
                               ImVec2(1, 1)))
        {
            if (m_SceneState == SceneState::Edit)
            {
                // Start playing
                m_SceneState = SceneState::Play;
            }
            else
            {
                // Stop playing
                m_SceneState = SceneState::Edit;
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
            m_Framebuffer->Resize(static_cast<uint32_t>(newSize.x), static_cast<uint32_t>(newSize.y));
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

        ImGui::PopStyleVar(2);
        ImGui::End();
    }

    void EditorLayer::RenderViewportImage()
    {
        m_Framebuffer->Resolve();
        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        ImGui::Image(m_Framebuffer->GetColorAttachment(), viewportSize, ImVec2(0, 1), ImVec2(1, 0));
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
        Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
        if (!selectedEntity || m_GizmoType == -1)
            return;

        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();

        float windowWidth = static_cast<float>(ImGui::GetWindowWidth());
        float windowHeight = static_cast<float>(ImGui::GetWindowHeight());
        ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

        const glm::mat4& cameraProjection = m_EditorCamera.GetProjection();
        glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();

        auto& tc = selectedEntity.GetComponent<TransformComponent>();
        glm::mat4 transform = tc.GetTransform();

        bool snap = Input::IsKeyPressed(Key::LeftControl);
        float snapValue = (m_GizmoType == ImGuizmo::OPERATION::ROTATE) ? 45.0f : 0.5f;
        float snapValues[3] = {snapValue, snapValue, snapValue};

        ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
                             static_cast<ImGuizmo::OPERATION>(m_GizmoType), ImGuizmo::LOCAL, glm::value_ptr(transform),
                             nullptr, snap ? snapValues : nullptr);

        if (ImGuizmo::IsUsing())
        {
            glm::vec3 translation, rotation, scale;
            Math::DecomposeTransform(transform, translation, rotation, scale);

            glm::vec3 deltaRotation = rotation - tc.Rotation;
            tc.Translation = translation;
            tc.Rotation += deltaRotation;
            tc.Scale = scale;
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
                if (control)
                    NewScene();
                break;

            case KeyCode::O:
                if (control)
                    OpenScene();
                break;

            case KeyCode::S:
                if (control && shift)
                    SaveSceneAs();
                break;

            case Key::Q:
                m_GizmoType = -1;
                break;

            case Key::W:
                m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
                break;

            case Key::E:
                m_GizmoType = ImGuizmo::OPERATION::ROTATE;
                break;

            case Key::R:
                m_GizmoType = ImGuizmo::OPERATION::SCALE;
                break;
        }

        return false;
    }

    bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
    {
        if (e.GetMouseButton() == static_cast<int>(MouseButton::ButtonLeft))
        {
            if (m_ViewportHovered && !ImGuizmo::IsOver() && !Input::IsKeyPressed(Key::LeftAlt))
                m_SceneHierarchyPanel.SetSelectedEntity(m_HoveredEntity);
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
        m_SceneHierarchyPanel.SetContext(m_ActiveScene);
    }

    void EditorLayer::OpenScene(const std::filesystem::path& path)
    {
        m_ActiveScene = CreateRef<Scene>();
        m_ActiveScene->OnViewportResize(static_cast<uint32_t>(m_ViewportSize.x),
                                        static_cast<uint32_t>(m_ViewportSize.y));
        m_SceneHierarchyPanel.SetContext(m_ActiveScene);

        SceneSerializer serializer(m_ActiveScene);
        serializer.Deserialize(path.string());
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
            SceneSerializer serializer(m_ActiveScene);
            serializer.Serialize(filepath);
        }
    }

} // namespace Titan