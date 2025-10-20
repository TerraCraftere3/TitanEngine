#include "Sandbox2D.h"
#include <algorithm>
#include <glm/gtx/matrix_decompose.hpp>

bool EditTransformImGui(glm::mat4& transform)
{
    using namespace glm;

    vec3 scale, translation, skew;
    vec4 perspective;
    quat orientation;

    if (!decompose(transform, scale, orientation, translation, skew, perspective))
        return false;

    vec3 rotation = degrees(eulerAngles(orientation));

    bool changed = false;
    if (ImGui::DragFloat3("Position", &translation.x, 0.1f))
        changed = true;
    if (ImGui::DragFloat3("Rotation", &rotation.x, 0.5f))
        changed = true;
    if (ImGui::DragFloat3("Scale", &scale.x, 0.01f))
        changed = true;

    const float minScale = 1e-4f;
    scale.x = max(scale.x, minScale);
    scale.y = max(scale.y, minScale);
    scale.z = max(scale.z, minScale);

    if (changed)
    {
        quat q = quat(radians(rotation));
        mat4 rotMat = toMat4(q);
        mat4 transMat = translate(mat4(1.0f), translation);
        mat4 scaleMat = glm::scale(mat4(1.0f), scale);
        transform = transMat * rotMat * scaleMat;
    }

    return changed;
}

Sandbox2D::Sandbox2D() : Layer("Sandbox2D Test"), m_CameraController(1280.0f / 720.0f) {}

void Sandbox2D::OnAttach()
{
    m_Texture = Titan::Texture2D::Create("textures/uv_test.jpg");
    m_LogoTexture = Titan::Texture2D::Create("textures/google_logo.png");
}

void Sandbox2D::OnDetach() {}

void Sandbox2D::OnUpdate(Titan::Timestep ts)
{
    Titan::RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
    Titan::RenderCommand::Clear();

    m_CameraController.OnUpdate(ts);

    Titan::Renderer2D::BeginScene(m_CameraController.GetCamera());
    Titan::Renderer2D::DrawQuad(position, size, color);
    Titan::Renderer2D::EndScene();
}

void Sandbox2D::OnEvent(Titan::Event& event)
{
    m_CameraController.OnEvent(event);
}

void Sandbox2D::OnImGuiRender(ImGuiContext* ctx)
{
    ImGui::SetCurrentContext(ctx);
    ImGui::Begin("Test");
    ImGui::SeparatorText("Quad (Renderer2D)");
    ImGui::DragFloat2("Position", glm::value_ptr(position), 0.025f);
    ImGui::DragFloat2("Size", glm::value_ptr(size), 0.025f, 0.01f, 10.0f);
    ImGui::ColorEdit4("Color", glm::value_ptr(color));
    ImGui::End();
}