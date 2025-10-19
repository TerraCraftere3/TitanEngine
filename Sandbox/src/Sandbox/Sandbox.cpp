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
            TI_TRACE("Pressed Key {}", (char)keyEvent.GetKeyCode());
        }
    }

    virtual void OnImGuiRender(ImGuiContext* ctx) override
    {
        ImGui::SetCurrentContext(ctx);
        ImGui::Begin("Test");
        static char text[1024] = "Hello, world!";
        ImGui::InputText("Text Input", text, IM_ARRAYSIZE(text));
        static float fltest = 0.0f;
        ImGui::DragFloat("Float Input", &fltest, 0.01f, -1.0f, 1.0f);
        static float coltest[4] = {1.0f, 0.0f, 1.0f, 1.0f};
        ImGui::ColorEdit4("Color Input", coltest);
        ImGui::End();
    }
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