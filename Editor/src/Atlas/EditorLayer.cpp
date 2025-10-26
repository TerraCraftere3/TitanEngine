#include "EditorLayer.h"
#include <algorithm>
#include <glm/gtx/matrix_decompose.hpp>
#include <random>

namespace Titan
{

    EditorLayer::EditorLayer() : Layer("EditorLayer Test") {}

    void EditorLayer::OnAttach()
    {
        // Textures
        m_FirstTexture = Texture2D::Create("textures/checkerboard.png");
        m_SecondTexture = Texture2D::Create("textures/uv_test.jpg");
        m_WhiteTexture = Renderer2D::GetWhiteTexture();

        // Quads
        const int quadCount = 10'000;
        std::mt19937 rng(1337);
        std::uniform_real_distribution<float> posDist(-10.0f, 10.0f);
        std::uniform_real_distribution<float> sizeDist(0.1f, 0.5f);
        std::uniform_real_distribution<float> rotDist(0.0f, 360.0f);
        std::uniform_real_distribution<float> rotSpeedDist(1.0f, 15.0f);
        std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);
        std::uniform_int_distribution<int> texDist(0, 2); // 0 = checkerboard, 1 = logo, 2 = color
        std::uniform_int_distribution<int> rotateDist(0, 1);

        m_ActiveScene = CreateRef<Scene>();

        for (int i = 0; i < quadCount; i++)
        {
            auto quad = m_ActiveScene->CreateEntity("Quad #" + std::to_string(i));
            auto& transform = quad.GetComponent<TransformComponent>();
            transform.Translation = glm::vec3(posDist(rng), posDist(rng), posDist(rng));
            transform.Rotation = glm::vec3(0.0f, 0.0f, rotDist(rng));
            transform.Scale = glm::vec3(sizeDist(rng), sizeDist(rng), 1.0f);

            int texChoice = texDist(rng);
            SpriteRendererComponent& sprite = quad.AddComponent<SpriteRendererComponent>();
            sprite.Color = {colorDist(rng), colorDist(rng), colorDist(rng), colorDist(rng)};
            if (texChoice == 0)
                sprite.Tex = m_FirstTexture;
            else if (texChoice == 1)
                sprite.Tex = m_SecondTexture;
            else
                sprite.Tex = m_WhiteTexture;
        }

        // Setup
        Application::GetInstance()->GetWindow().SetVSync(false);

        FramebufferSpecification fbSpec;
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        m_Framebuffer = Framebuffer::Create(fbSpec);

        class CameraController : public ScriptableEntity
        {
        public:
            void OnCreate() {}

            void OnDestroy() {}

            void OnUpdate(Timestep ts)
            {
                auto& translation = GetComponent<TransformComponent>().Translation;
                float speed = 5.0f;

                if (Input::IsKeyPressed(KeyCode::A))
                    translation.x -= speed * ts;
                if (Input::IsKeyPressed(KeyCode::D))
                    translation.x += speed * ts;
                if (Input::IsKeyPressed(KeyCode::W))
                    translation.y += speed * ts;
                if (Input::IsKeyPressed(KeyCode::S))
                    translation.y -= speed * ts;
                /*if (Input::IsKeyPressed(KeyCode::Space))
                    translation.z += speed * ts;
                if (Input::IsKeyPressed(KeyCode::LeftShift))
                    translation.z -= speed * ts;*/
            }
        };

        // Cameras
        {
            auto orthographicCam = m_ActiveScene->CreateEntity("Orthographic Camera");
            auto& cc = orthographicCam.AddComponent<CameraComponent>();
            cc.Camera.SetProjectionType(SceneCamera::ProjectionType::Orthographic);
            cc.Primary = false;

            orthographicCam.AddComponent<NativeScriptComponent>().Bind<CameraController>();
        }
        {
            auto perspectiveCam = m_ActiveScene->CreateEntity("Perspective Camera");
            auto& cc = perspectiveCam.AddComponent<CameraComponent>();
            cc.Camera.SetProjectionType(SceneCamera::ProjectionType::Perspective);

            perspectiveCam.AddComponent<NativeScriptComponent>().Bind<CameraController>();
        }
        m_SceneHierarchyPanel.SetContext(m_ActiveScene);
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

    void EditorLayer::OnEvent(Event& event)
    {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<KeyPressedEvent>(TI_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
    }

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
                if (ImGui::MenuItem("New", "Ctrl+N"))
                    NewScene();
                if (ImGui::MenuItem("Open...", "Ctrl+O"))
                    OpenScene();
                if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
                    SaveSceneAs();
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        ImGui::End();

        m_SceneHierarchyPanel.OnImGuiRender();

        ImGui::Begin("Statistics");
        auto stats = Renderer2D::GetStats();
        ImGui::Text("Draw Calls: %d", stats.GetTotalDrawCalls());
        ImGui::Text("Quads: %d", stats.GetTotalQuadCount());
        ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
        ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
        ImGui::Text("Triangles: %d", stats.GetTotalTriangleCount());
        ImGui::Text("FPS: %.1f", m_FPS);
        ImGui::End();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0)); // Remove padding
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);      // Remove border

        ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        if (m_ViewportSize != *((glm::vec2*)&viewportPanelSize))
        {
            m_Framebuffer->Resize((uint32_t)viewportPanelSize.x, (uint32_t)viewportPanelSize.y);
            m_ViewportSize = {viewportPanelSize.x, viewportPanelSize.y};
            m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        }
        ImGui::Image(m_Framebuffer->GetColorAttachment(), viewportPanelSize, ImVec2(0, 1), ImVec2(1, 0));

        ImGui::End();
        ImGui::PopStyleVar(2);
    }

    bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
    {
        // Shortcuts
        if (e.GetRepeatCount() > 0)
            return false;

        bool control = Input::IsKeyPressed(KeyCode::LeftControl) || Input::IsKeyPressed(KeyCode::RightControl);
        bool shift = Input::IsKeyPressed(KeyCode::LeftShift) || Input::IsKeyPressed(KeyCode::RightShift);
        switch (e.GetKeyCode())
        {
            case KeyCode::N:
            {
                if (control)
                    NewScene();

                break;
            }
            case KeyCode::O:
            {
                if (control)
                    OpenScene();

                break;
            }
            case KeyCode::S:
            {
                if (control && shift)
                    SaveSceneAs();

                break;
            }
        }
    }

    void EditorLayer::NewScene()
    {
        m_ActiveScene = CreateRef<Scene>();
        m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        m_SceneHierarchyPanel.SetContext(m_ActiveScene);
    }

    void EditorLayer::OpenScene()
    {
        std::string filepath = FileDialogs::OpenFile("Titan Scene (*.titan)\0*.titan\0");
        if (!filepath.empty())
        {
            m_ActiveScene = CreateRef<Scene>();
            m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            m_SceneHierarchyPanel.SetContext(m_ActiveScene);

            SceneSerializer serializer(m_ActiveScene);
            serializer.Deserialize(filepath);
        }
    }

    void EditorLayer::SaveSceneAs()
    {
        std::string filepath = FileDialogs::SaveFile("Titan Scene (*.titan)\0*.titan\0");
        if (!filepath.empty())
        {
            SceneSerializer serializer(m_ActiveScene);
            serializer.Serialize(filepath);
        }
    }
} // namespace Titan