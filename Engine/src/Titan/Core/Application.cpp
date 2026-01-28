#include "Titan/Core/Application.h"
#include "Titan/Core/Input.h"
#include "Titan/Core/KeyCodes.h"
#include "Titan/Core/Log.h"
#include "Titan/ImGuiLayer.h"
#include "Titan/PCH.h"
#include "Titan/Physics/Physics3D/Physics3D.h"
#include "Titan/Project/Project.h"
#include "Titan/Renderer/RHI/PipelineState.h"
#include "Titan/Renderer/RenderCommand.h"
#include "Titan/Renderer/Renderer.h"
#include "Titan/Scene/Assets.h"
#include "Titan/Scripting/ScriptEngine.h"
#include "Titan/Utils/Time.h"
// clang-format off
#ifdef APIENTRY
    #undef APIENTRY
#endif
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Application.h"
// clang-format on
namespace Titan
{

    Application* Application::s_Instance = nullptr;

    Application::Application(const ApplicationSpecification& specification) : m_Specification(specification)
    {
        TI_CORE_ASSERT(!s_Instance, "Application already exists! There can only be one");
        s_Instance = this;

        m_Window = Scope<Window>(Window::Create(specification.WindowProperties));
        m_Window->SetEventCallback(TI_BIND_EVENT_FN(Application::OnEvent));

        std::filesystem::path projectDir =
            std::filesystem::path(specification.CommandLineArgs.Args[1]) / "project.tiproj";

        Project::Load(projectDir);

        Assets::Init();
        Renderer::Init();
        ScriptEngine::Init();
        Physics3D::Init();

        m_ImGuiLayer = new ImGuiLayer(specification.Theme);
        PushOverlay(m_ImGuiLayer);

        m_ImGuiEnabled = !specification.DisableImGui;
    }

    Application::~Application()
    {
        ScriptEngine::Shutdown();
        Renderer::Shutdown();
        Physics3D::Shutdown();
        Assets::Shutdown();
    }

    void Application::Close()
    {
        m_Running = false;
    }

    void Application::Run()
    {
        while (m_Running)
        {
            TI_PROFILE_BEGIN_FRAME();
            float time = static_cast<float>(GetCurrentTimeSeconds());
            Timestep timestep = time - m_LastFrameTime;
            m_LastFrameTime = time;

            RenderCommand::BeginFrame();

            {
                TI_PROFILE_SCOPE("Main Thread Queue");
                ExecuteMainThreadQueue();
            }

            if (!m_Minimized)
            {
                TI_PROFILE_SCOPE("Layer Update");
                for (Layer* layer : m_LayerStack)
                    layer->OnUpdate(timestep);
            }

            if (m_ImGuiEnabled)
            {
                TI_PROFILE_SCOPE("UI Update");
                m_ImGuiLayer->Begin();
                for (Layer* layer : m_LayerStack)
                {
                    layer->OnImGuiRender(ImGui::GetCurrentContext());
                }
                m_ImGuiLayer->End();
            }

            {
                TI_PROFILE_SCOPE("Window Update");
                m_Window->OnUpdate();
            }

            RenderCommand::EndFrame();
            TI_PROFILE_END_FRAME();
        }
    }

    void Application::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(TI_BIND_EVENT_FN(Application::OnWindowClosed));
        dispatcher.Dispatch<WindowResizeEvent>(TI_BIND_EVENT_FN(Application::OnWindowResize));

        for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
        {
            (*--it)->OnEvent(e);
            if (e.Handled)
                break;
        }
    }

    void Application::PushLayer(Layer* layer)
    {
        m_LayerStack.PushLayer(layer);
    }

    void Application::PushOverlay(Layer* layer)
    {
        m_LayerStack.PushOverlay(layer);
    }

    void Application::UpdateTheme(UITheme theme)
    {
        m_ImGuiLayer->SetTheme(theme);
    }

    bool Application::OnWindowResize(WindowResizeEvent& e)
    {
        if (e.GetWidth() == 0 || e.GetHeight() == 0)
        {
            m_Minimized = true;
            return false;
        }

        m_Minimized = false;
        Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

        return false;
    }

    void Application::ExecuteMainThreadQueue()
    {
        std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

        for (auto& func : m_MainThreadQueue)
            func();

        m_MainThreadQueue.clear();
    }

    void Application::SubmitToMainThread(const std::function<void()>& function)
    {
        std::scoped_lock<std::mutex> lock(m_MainThreadQueueMutex);

        m_MainThreadQueue.emplace_back(function);
    }

    bool Application::OnWindowClosed(WindowCloseEvent& e)
    {
        m_Running = false;
        return true;
    }

    void TI_API Titan::DeleteApplication(Application* app)
    {
        delete app;
    }

} // namespace Titan