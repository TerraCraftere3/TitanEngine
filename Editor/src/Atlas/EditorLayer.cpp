#include "EditorLayer.h"
#include <algorithm>
#include <glm/gtx/matrix_decompose.hpp>
#include <random>

namespace Titan
{

    EditorLayer::EditorLayer() : Layer("EditorLayer Test") {}

#define TEST_QUADS_COUNT 10000
#define TEST_QUADS_HAS_ROTATED 1

    void EditorLayer::OnAttach()
    {
        m_FirstTexture = Texture2D::Create("textures/checkerboard.png");
        m_SecondTexture = Texture2D::Create("textures/uv_test.jpg");

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

        m_ActiveScene = CreateRef<Scene>();
        auto cam = m_ActiveScene->CreateEntity("Camera Entity");
        cam.AddComponent<CameraComponent>(glm::ortho(-16.0f, 16.0f, -9.0f, 9.0f, -1.0f, FLT_MAX));

        auto secondCam = m_ActiveScene->CreateEntity("Clip-Space Entity");
        auto& cc = secondCam.AddComponent<CameraComponent>(glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f));
        cc.Primary = false;

        for (int i = 0; i < quadCount; i++)
        {
            auto quad = m_ActiveScene->CreateEntity("Quad #" + std::to_string(quadCount));
            auto& transform = quad.GetComponent<TransformComponent>();
            glm::mat4 matrix = glm::mat4(1.0f);
            matrix = glm::translate(matrix, glm::vec3(posDist(rng), posDist(rng), posDist(rng)));
#if TEST_QUADS_HAS_ROTATED == 1
            matrix = glm::rotate(matrix, glm::radians(rotDist(rng)), glm::vec3(0.0f, 0.0f, 1.0f));
#endif
            matrix = glm::scale(matrix, glm::vec3(sizeDist(rng), sizeDist(rng), 1.0f));
            transform.Transform = matrix;
            /*if (texChoice == 0)
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
            m_Quads.push_back(quad);*/
            auto& sprite = quad.AddComponent<SpriteRendererComponent>();
            sprite.Color = {colorDist(rng), colorDist(rng), colorDist(rng), colorDist(rng)};
        }

        Application::GetInstance()->GetWindow().SetVSync(false);

        FramebufferSpecification fbSpec;
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        m_Framebuffer = Framebuffer::Create(fbSpec);
    }

    void EditorLayer::OnDetach() {}

    void EditorLayer::OnUpdate(Timestep ts)
    {
        if (ts.GetSeconds() > 0.0f)
            m_FPS = 1.0f / ts.GetSeconds();

        m_Framebuffer->Bind();
        RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1});
        RenderCommand::Clear();

        Renderer2D::ResetStats();
        m_ActiveScene->OnUpdate(ts);
        m_Framebuffer->Unbind();
    }

    void EditorLayer::OnEvent(Event& event) {}

    void EditorLayer::OnImGuiRender(ImGuiContext* ctx)
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
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                        ImGuiWindowFlags_NoMove;
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
                    Application::GetInstance()->Close();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        ImGui::End();

        ImGui::Begin("Test");
        ImGui::SeparatorText("Renderer");
        auto stats = Renderer2D::GetStats();
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

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0)); // Remove padding
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);      // Remove border

        ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        if (m_ViewportSize != *((glm::vec2*)&viewportPanelSize))
        {
            m_Framebuffer->Resize((uint32_t)viewportPanelSize.x, (uint32_t)viewportPanelSize.y);
            m_ViewportSize = {viewportPanelSize.x, viewportPanelSize.y};
        }
        ImGui::Image(m_Framebuffer->GetColorAttachment(), viewportPanelSize, ImVec2(0, 1), ImVec2(1, 0));

        ImGui::End();
        ImGui::PopStyleVar(2);
    }
} // namespace Titan