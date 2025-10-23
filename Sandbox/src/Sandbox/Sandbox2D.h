#pragma once
#include <Titan.h>
class Sandbox2D : public Titan::Layer
{
public:
    Sandbox2D();

    virtual void OnAttach() override;
    virtual void OnDetach() override;

    virtual void OnUpdate(Titan::Timestep ts) override;
    virtual void OnEvent(Titan::Event& event) override;
    virtual void OnImGuiRender(ImGuiContext* ctx) override;

private:
    float m_FPS = 0.0f;
    glm::vec2 position = glm::vec2(0.0f);
    glm::vec2 size = glm::vec2(1.0f);
    glm::vec3 rotation = glm::vec3(0.0f);
    Titan::Ref<Titan::Texture2D> m_FirstTexture, m_SecondTexture;
    Titan::Ref<Titan::Framebuffer> m_Framebuffer;

    Titan::OrthographicCameraController m_CameraController;

    struct ProfileResult
    {
        const char* Name;
        float Time;
    };

    std::vector<ProfileResult> m_ProfileResults;

    struct QuadData
    {
        glm::vec2 Position;
        glm::vec2 Size;
        glm::vec3 Rotation;
        float RotationSpeed;
        bool HasTexture;
        Titan::Ref<Titan::Texture2D> Texture;
        glm::vec4 Color;
    };
    std::vector<QuadData> m_Quads;
};