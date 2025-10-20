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
    glm::vec2 position = glm::vec2(0.0f);
    glm::vec2 size = glm::vec2(1.0f, 1.0f);
    Titan::Ref<Titan::Texture2D> m_CheckerboardTexture, m_LogoTexture;

    Titan::OrthographicCameraController m_CameraController;
};