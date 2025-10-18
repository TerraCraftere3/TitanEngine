#include <Titan.h>

class ExampleLayer : public Titan::Layer
{
public:
    ExampleLayer() : Layer("Example") {}

    void OnUpdate() override {}

    void OnEvent(Titan::Event& event) override {}
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