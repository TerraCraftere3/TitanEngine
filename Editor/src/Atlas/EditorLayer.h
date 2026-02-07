#pragma once
#include <Titan/Core/Layer.h>
#include <Titan/Core/Timer.h>
#include <Titan/Events/Event.h>
#include <Titan/Events/KeyEvent.h>
#include <Titan/Events/MouseEvent.h>
#include <Titan/Renderer/Mesh.h>
#include <Titan/Renderer/RHI/Framebuffer.h>
#include <Titan/Renderer/RHI/Texture.h>
#include <Titan/Renderer/Systems/SceneRenderer.h>
#include <Titan/Scene/Scene.h>
#include "EditorProperties.h"

namespace Titan
{
    class SceneHierarchyPanel;
    class ContentBrowserPanel;
    class LogPanel;
    class ProfilerPanel;
    class EditorLayer : public Layer
    {
    public:
        EditorLayer();

        virtual void OnAttach() override;
        virtual void OnDetach() override;

        virtual void OnUpdate(Timestep ts) override;
        virtual void OnEvent(Event& event) override;
        virtual void OnImGuiRender(ImGuiContext* ctx) override;

    private:
        bool OnKeyPressed(KeyPressedEvent& e);
        bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

        void NewScene();
        void OpenScene();
        void OpenScene(const std::filesystem::path& path);
        void SaveScene();
        void SaveSceneAs();

        void RenderDockspace();
        void RenderMenuBar();
        void RenderStatisticsPanel(bool* open = nullptr);
        void RenderViewport(bool* open = nullptr);

        void UpdateViewportBounds();
        void HandleViewportResize();
        void RenderSceneControlToolbar();
        void RenderGizmoToolbar();
        void RenderViewportImage();
        void HandleSceneDragDrop();
        void HandleGizmoManipulation();

        void SerializeScene(Ref<Scene> scene, const std::filesystem::path& path);
        void OnScenePlay();
        void OnSceneSimulate();
        void OnSceneStop();

        void OnDuplicateEntity();

    private:
        enum class SceneState
        {
            Edit = 0,
            Play = 1,
            Simulate = 2
        };

        // State
        SceneState m_SceneState = SceneState::Edit;
        float m_FPS = 0.0f;
        int m_GizmoType = -1;

        // Viewport State
        bool m_EnableRender = true;
        bool m_ViewportFocused = false;
        bool m_ViewportHovered = false;
        glm::vec2 m_ViewportSize = {0.0f, 0.0f};
        glm::vec2 m_ViewportBounds[2];
        ImVec2 m_ViewportImagePos;
        ImVec2 m_ViewportImageSize;

        // Multi-view state
        int m_ActiveViewportIndex = 0;
        ImVec2 m_SubViewportImagePos[4] = {};
        ImVec2 m_SubViewportImageSize[4] = {};
        bool m_SubViewportHovered[4] = {false, false, false, false};
        bool m_EnableMultiViewports = true;

        // Resources
        Ref<Texture2D> m_StartIcon;
        Ref<Texture2D> m_SimulateIcon;
        Ref<Texture2D> m_StopIcon;

        Ref<Mesh> m_DragonMesh;

        // Scene
        Ref<Scene> m_ActiveScene;
        Ref<Scene> m_EditorScene;
        std::filesystem::path m_EditorScenePath;
        EditorCamera m_EditorCameras[4];
        Ref<SceneRenderer> m_SceneRenderers[4];

        // Panels
        Scope<SceneHierarchyPanel> m_SceneHierarchyPanel;
        Scope<ContentBrowserPanel> m_ContentBrowserPanel;
        Scope<LogPanel> m_LogPanel;
        Scope<ProfilerPanel> m_ProfilerPanel;

        // Editor properties
        EditorProperties m_EditorProperties;
    };
} // namespace Titan