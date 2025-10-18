#include <Titan.h>

class ExampleLayer : public Titan::Layer
{
public:
    ExampleLayer() : Layer("Example") {}

    void OnUpdate() override {}

    void OnEvent(Titan::Event& event) override
    {
        if (event.GetEventType() == Titan::EventType::KeyPressed)
        {
            auto keyEvent = (Titan::KeyPressedEvent&)event;
            LOG_TRACE("Pressed Key {}", (char)keyEvent.GetKeyCode());
        }
    }
};

class Sandbox : public Titan::Application
{
public:
    Sandbox()
    {
        PushLayer(new ExampleLayer());
        PushOverlay(new Titan::ImGuiLayer());
    }
    ~Sandbox() {}
};

Titan::Application* Titan::CreateApplication()
{
    return new Sandbox();
}