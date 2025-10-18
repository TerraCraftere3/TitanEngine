#include <Titan.h>

class ExampleLayer : public Titan::Layer
{
public:
    ExampleLayer() : Layer("Example") {}

    void OnUpdate() override
    {
        // LOG_TRACE("ExampleLayer::Update");
    }

    void OnEvent(Titan::Event& event) override { LOG_TRACE("{0}", event.ToString()); }
};

class Sandbox : public Titan::Application
{
public:
    Sandbox() { PushLayer(new ExampleLayer()); }
    ~Sandbox() {}
};

Titan::Application* Titan::CreateApplication()
{
    return new Sandbox();
}