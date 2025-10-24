#pragma once
#include <Titan.h>

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
        float m_FPS = 0.0f;
        glm::vec2 position = glm::vec2(0.0f);
        glm::vec2 size = glm::vec2(1.0f);
        glm::vec3 rotation = glm::vec3(0.0f);
        Ref<Texture2D> m_FirstTexture, m_SecondTexture;
        Ref<Framebuffer> m_Framebuffer;

        OrthographicCameraController m_CameraController;

        struct ProfileResult
        {
            const char* Name;
            float Time;
        };

        std::vector<ProfileResult> m_ProfileResults;

        struct QuadData
        {
            glm::vec3 Position;
            glm::vec2 Size;
            glm::vec3 Rotation;
            float RotationSpeed;
            bool HasTexture;
            Ref<Texture2D> Texture;
            glm::vec4 Color;
        };
        std::vector<QuadData> m_Quads;
        glm::vec2 m_ViewportSize = {0.0f, 0.0f};
    };
} // namespace Titan