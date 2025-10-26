#pragma once
#include <Titan.h>
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
        void SaveSceneAs();

    private:
        float m_FPS = 0.0f;
        Ref<Texture2D> m_FirstTexture, m_SecondTexture, m_WhiteTexture;
        Ref<Framebuffer> m_Framebuffer;
        int m_GizmoType = -1;

        struct ProfileResult
        {
            const char* Name;
            float Time;
        };

        std::vector<ProfileResult> m_ProfileResults;

        bool m_ViewportFocused = false, m_ViewportHovered = false;
        glm::vec2 m_ViewportSize = {0.0f, 0.0f};
        glm::vec2 m_ViewportBounds[2];

        Ref<Scene> m_ActiveScene;
        Entity m_HoveredEntity;
        EditorCamera m_EditorCamera;

        // Panels
        SceneHierarchyPanel m_SceneHierarchyPanel;
    };
} // namespace Titan