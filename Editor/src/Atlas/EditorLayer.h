#pragma once
#include <Titan/Core/Layer.h>
#include <Titan/Core/Timer.h>
#include <Titan/Events/Event.h>
#include <Titan/Events/KeyEvent.h>
#include <Titan/Events/MouseEvent.h>
#include <Titan/Renderer/Framebuffer.h>
#include <Titan/Renderer/Texture.h>
#include <Titan/Scene/Scene.h>
#include "Panels/ContentBrowserPanel.h"
#include "Panels/SceneHierarchyPanel.h"

namespace Titan
{
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
        // Event Handlers
        bool OnKeyPressed(KeyPressedEvent& e);
        bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

        // Scene Management
        void NewScene();
        void OpenScene();
        void OpenScene(const std::filesystem::path& path);
        void SaveSceneAs();

        // Update Methods
        void UpdateHoveredEntity();

        // ImGui Rendering Methods
        void RenderDockspace();
        void RenderMenuBar();
        void RenderStatisticsPanel();
        void RenderViewport();

        // Viewport Helper Methods
        void UpdateViewportBounds();
        void HandleViewportResize();
        void RenderSceneControlToolbar();
        void RenderGizmoToolbar();
        void RenderViewportImage();
        void HandleSceneDragDrop();
        void HandleGizmoManipulation();

    private:
        enum class SceneState
        {
            Edit = 0,
            Play = 1
        };

        // State
        SceneState m_SceneState = SceneState::Edit;
        float m_FPS = 0.0f;
        int m_GizmoType = -1;

        // Viewport State
        bool m_ViewportFocused = false;
        bool m_ViewportHovered = false;
        glm::vec2 m_ViewportSize = {0.0f, 0.0f};
        glm::vec2 m_ViewportBounds[2];
        ImVec2 m_ViewportImagePos;
        ImVec2 m_ViewportImageSize;

        // Resources
        Ref<Framebuffer> m_Framebuffer;
        Ref<Texture2D> m_StartIcon;
        Ref<Texture2D> m_StopIcon;

        // Scene
        Ref<Scene> m_ActiveScene;
        Entity m_HoveredEntity;
        EditorCamera m_EditorCamera;

        // Panels
        SceneHierarchyPanel m_SceneHierarchyPanel;
        ContentBrowserPanel m_ContentBrowserPanel;
    };
} // namespace Titan