#include <Titan.h>

#include <glm/ext/matrix_clip_space.hpp> // glm::perspective
#include <glm/ext/matrix_transform.hpp>  // glm::translate, glm::rotate, glm::scale
#include <glm/ext/scalar_constants.hpp>  // glm::pi
#include <glm/mat4x4.hpp>                // glm::mat4
#include <glm/vec3.hpp>                  // glm::vec3
#include <glm/vec4.hpp>                  // glm::vec4

glm::mat4 camera(float Translate, glm::vec2 const& Rotate)
{
    glm::mat4 Projection = glm::perspective(glm::pi<float>() * 0.25f, 4.0f / 3.0f, 0.1f, 100.f);
    glm::mat4 View = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -Translate));
    View = glm::rotate(View, Rotate.y, glm::vec3(-1.0f, 0.0f, 0.0f));
    View = glm::rotate(View, Rotate.x, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
    return Projection * View * Model;
}

class ExampleLayer : public Titan::Layer
{
public:
    ExampleLayer() : Layer("Example")
    {
        auto cam = camera(5.0f, {0.5f, 0.5f});
        LOG_TRACE("Math Test: {}", cam);
    }

    void OnUpdate() override {}

    void OnEvent(Titan::Event& event) override
    {
        if (event.GetEventType() == Titan::EventType::KeyPressed)
        {
            auto keyEvent = (Titan::KeyPressedEvent&)event;
            LOG_TRACE("Pressed Key {}", (char)keyEvent.GetKeyCode());
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