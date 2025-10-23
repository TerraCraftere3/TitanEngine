#include "Sandbox2D.h"
#include <algorithm>
#include <glm/gtx/matrix_decompose.hpp>
#include <random>

Sandbox2D::Sandbox2D() : Layer("Sandbox2D Test"), m_CameraController(1280.0f / 720.0f) {}

#define TEST_QUADS_COUNT 10000
#define TEST_QUADS_HAS_ROTATED 0

void Sandbox2D::OnAttach()
{
    m_FirstTexture = Titan::Texture2D::Create("textures/checkerboard.png");
    m_SecondTexture = Titan::Texture2D::Create("textures/uv_test.jpg");

    // Precompute quads
    const int quadCount = TEST_QUADS_COUNT; // how many quads to render
    std::mt19937 rng(1337);                 // fixed seed for consistency
    std::uniform_real_distribution<float> posDist(-10.0f, 10.0f);
    std::uniform_real_distribution<float> sizeDist(0.1f, 0.5f);
    std::uniform_real_distribution<float> rotDist(0.0f, 360.0f);
    std::uniform_real_distribution<float> rotSpeedDist(1.0f, 15.0f);
    std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);
    std::uniform_int_distribution<int> texDist(0, 2); // 0 = checkerboard, 1 = logo, 2 = color
#if TEST_QUADS_HAS_ROTATED == 1
    std::uniform_int_distribution<int> rotateDist(0, 1);
#endif

    m_Quads.clear();
    for (int i = 0; i < quadCount; i++)
    {
        QuadData quad;
        quad.Position = {posDist(rng), posDist(rng)};
        quad.Size = {sizeDist(rng), sizeDist(rng)};
#if TEST_QUADS_HAS_ROTATED == 1
        quad.Rotation = {0.0f, 0.0f, rotDist(rng)};
        quad.RotationSpeed = rotSpeedDist(rng);
#endif
        int texChoice = texDist(rng);
        if (texChoice == 0)
        {
            quad.HasTexture = true;
            quad.Texture = m_FirstTexture;
            quad.Color = {colorDist(rng), colorDist(rng), colorDist(rng), colorDist(rng)};
        }
        else if (texChoice == 1)
        {
            quad.HasTexture = true;
            quad.Texture = m_SecondTexture;
            quad.Color = {colorDist(rng), colorDist(rng), colorDist(rng), colorDist(rng)};
        }
        else
        {
            quad.HasTexture = false;
            quad.Color = {colorDist(rng), colorDist(rng), colorDist(rng), colorDist(rng)};
        }
        m_Quads.push_back(quad);
    }

    m_CameraController.SetZoomLevel(3.0f);

    Titan::Application::GetInstance()->GetWindow().SetVSync(false);
}

void Sandbox2D::OnDetach() {}

void Sandbox2D::OnUpdate(Titan::Timestep ts)
{
    if (ts.GetSeconds() > 0.0f)
        m_FPS = 1.0f / ts.GetSeconds();

    m_CameraController.OnUpdate(ts);

    Titan::RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
    Titan::RenderCommand::Clear();

    Titan::Renderer2D::ResetStats();
    Titan::Renderer2D::BeginScene(m_CameraController.GetCamera());

#if TEST_QUADS_HAS_ROTATED == 1
    for (auto& quad : m_Quads)
    {
        quad.Rotation += glm::vec3(0.0f, 0.0f, ts * quad.RotationSpeed);
        if (quad.HasTexture)
            Titan::Renderer2D::DrawRotatedQuad(quad.Position + position, quad.Size * size, quad.Rotation + rotation,
                                               quad.Texture, 1.0f, quad.Color);
        else
            Titan::Renderer2D::DrawRotatedQuad(quad.Position + position, quad.Size * size, quad.Rotation + rotation,
                                               quad.Color);
    }
#else
    for (auto& quad : m_Quads)
    {
        if (quad.HasTexture)
            Titan::Renderer2D::DrawQuad(quad.Position + position, quad.Size * size, quad.Texture, 1.0f, quad.Color);
        else
            Titan::Renderer2D::DrawQuad(quad.Position + position, quad.Size * size, quad.Color);
    }
#endif

    Titan::Renderer2D::EndScene();
}

void Sandbox2D::OnEvent(Titan::Event& event)
{
    m_CameraController.OnEvent(event);
}

void Sandbox2D::OnImGuiRender(ImGuiContext* ctx)
{
    ImGui::SetCurrentContext(ctx);
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |=
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    ImGuiIO& io = ImGui::GetIO();
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

    // Show demo options and help
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Exit", "Alt+F4"))
            {
                Titan::Application::GetInstance()->Close();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::End();

    ImGui::Begin("Test");
    ImGui::SeparatorText("Camera");
    ImGui::Text("Zoom: %f", m_CameraController.GetZoomLevel());
    ImGui::SeparatorText("Renderer");
    auto stats = Titan::Renderer2D::GetStats();
    ImGui::Text("Draw Calls: %d", stats.GetTotalDrawCalls());
    ImGui::Text("Quads: %d", stats.GetTotalQuadCount());
    ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
    ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
    ImGui::Text("Triangles: %d", stats.GetTotalTriangleCount());
    ImGui::Text("FPS: %.1f", m_FPS);
    ImGui::SeparatorText("Quads");
    ImGui::DragFloat2("Position", glm::value_ptr(position));
    ImGui::DragFloat2("Size", glm::value_ptr(size), 0.05f, 0.1f, 1.5f);
    ImGui::DragFloat3("Rotation", glm::value_ptr(rotation), 1.0f, -FLT_MAX, FLT_MAX, "%.0f deg");
    ImGui::SeparatorText("Textures");
    ImGui::Image(m_FirstTexture->GetNativeTexture(), {128, 128}, ImVec2(0, 1), ImVec2(1, 0));
    ImGui::SameLine();
    ImGui::Image(m_SecondTexture->GetNativeTexture(), {128, 128}, ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();
}
