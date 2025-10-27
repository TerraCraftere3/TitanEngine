#pragma once
#include <Titan.h>
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
        bool OnKeyPressed(KeyPressedEvent& e);
        bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

        void NewScene();
        void OpenScene();
        void OpenScene(const std::filesystem::path& path);
        void SaveSceneAs();

        void OnScenePlay();
        void OnSceneStop();

        void RenderDockspace();
        void RenderViewport();
        void RenderToolbar();

    private:
        float m_FPS = 0.0f;
        Ref<Framebuffer> m_Framebuffer;

        enum class SceneState
        {
            Edit = 0,
            Play = 1
        };
        SceneState m_SceneState = SceneState::Edit;

        int m_GizmoType = -1;
        bool m_ViewportFocused = false, m_ViewportHovered = false;
        glm::vec2 m_ViewportSize = {0.0f, 0.0f};
        glm::vec2 m_ViewportBounds[2];

        Ref<Texture2D> m_StartIcon, m_StopIcon;
        Ref<Scene> m_ActiveScene;
        Entity m_HoveredEntity;
        EditorCamera m_EditorCamera;

        // Panels
        SceneHierarchyPanel m_SceneHierarchyPanel;
        ContentBrowserPanel m_ContentBrowserPanel;
    };
} // namespace Titan