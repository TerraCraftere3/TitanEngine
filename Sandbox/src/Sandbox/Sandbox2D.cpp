#include "Sandbox2D.h"
#include <algorithm>
#include <glm/gtx/matrix_decompose.hpp>
#include <random>

Sandbox2D::Sandbox2D() : Layer("Sandbox2D Test"), m_CameraController(1280.0f / 720.0f) {}

#define TEST_QUADS_COUNT 5000
#define TEST_QUADS_HAS_ROTATED 1

void Sandbox2D::OnAttach()
{
    m_CheckerboardTexture = Titan::Texture2D::Create("textures/checkerboard.png");
    m_LogoTexture = Titan::Texture2D::Create("textures/google_logo.png");

    // Precompute quads
    const int quadCount = TEST_QUADS_COUNT; // how many quads to render
    std::mt19937 rng(1337);                 // fixed seed for consistency
    std::uniform_real_distribution<float> posDist(-10.0f, 10.0f);
    std::uniform_real_distribution<float> sizeDist(0.1f, 0.5f);
    std::uniform_real_distribution<float> rotDist(0.0f, 360.0f);
    std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);
    std::uniform_int_distribution<int> texDist(0, 2); // 0 = checkerboard, 1 = logo, 2 = color
    std::uniform_int_distribution<int> rotateDist(0, 1);

    m_Quads.clear();
    for (int i = 0; i < quadCount; i++)
    {
        QuadData quad;
        quad.Position = {posDist(rng), posDist(rng)};
        quad.Size = {sizeDist(rng), sizeDist(rng)};
        quad.Rotation = {0.0f, 0.0f, rotDist(rng)};
#if TEST_QUADS_HAS_ROTATED == 1
        quad.isRotated = rotateDist(rng) == 1;
#else
        quad.isRotated = false;
#endif
        int texChoice = texDist(rng);
        if (texChoice == 0)
        {
            quad.HasTexture = true;
            quad.Texture = m_CheckerboardTexture;
            quad.Color = {colorDist(rng), colorDist(rng), colorDist(rng), 1.0f};
        }
        else if (texChoice == 1)
        {
            quad.HasTexture = true;
            quad.Texture = m_LogoTexture;
            quad.Color = {colorDist(rng), colorDist(rng), colorDist(rng), 1.0f};
        }
        else
        {
            quad.HasTexture = false;
            quad.Color = {colorDist(rng), colorDist(rng), colorDist(rng), 1.0f};
        }
        m_Quads.push_back(quad);
    }

    m_CameraController.SetZoomLevel(3.0f);
}

void Sandbox2D::OnDetach() {}

void Sandbox2D::OnUpdate(Titan::Timestep ts)
{
    if (ts.GetSeconds() > 0.0f)
        m_FPS = 1.0f / ts.GetSeconds();

    m_CameraController.OnUpdate(ts);

    Titan::RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
    Titan::RenderCommand::Clear();

    Titan::Renderer2D::BeginScene(m_CameraController.GetCamera());

    for (auto& quad : m_Quads)
    {
        if (quad.isRotated)
            if (quad.HasTexture)
                Titan::Renderer2D::DrawRotatedQuad(quad.Position + position, quad.Size * size, quad.Rotation + rotation,
                                                   quad.Texture, 1.0f, quad.Color);
            else
                Titan::Renderer2D::DrawRotatedQuad(quad.Position, quad.Size, quad.Rotation, quad.Color);
        if (quad.HasTexture)
            Titan::Renderer2D::DrawQuad(quad.Position + position, quad.Size * size, quad.Texture, 1.0f, quad.Color);
        else
            Titan::Renderer2D::DrawQuad(quad.Position, quad.Size, quad.Color);
    }

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

    ImGui::SeparatorText("Camera");
    ImGui::Text("Zoom: %f", m_CameraController.GetZoomLevel());
    ImGui::SeparatorText("Renderer");
    ImGui::Text("Number of Quads: %d", (int)m_Quads.size());
    ImGui::Text("FPS: %.1f", m_FPS);
    ImGui::SeparatorText("Quads");
    ImGui::DragFloat3("Position", glm::value_ptr(position));
    ImGui::DragFloat3("Rotation", glm::value_ptr(position), 1.0f, -FLT_MAX, FLT_MAX, "%.0f deg");
    ImGui::DragFloat2("Size", glm::value_ptr(position));
    ImGui::SeparatorText("Textures");
    ImGui::Image(m_CheckerboardTexture->GetNativeTexture(), {128, 128}, ImVec2(0, 1), ImVec2(1, 0));
    ImGui::SameLine();
    ImGui::Image(m_LogoTexture->GetNativeTexture(), {128, 128}, ImVec2(0, 1), ImVec2(1, 0));

    ImGui::End();
}
