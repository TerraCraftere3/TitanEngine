#include "Titan/Core/Application.h"
#include "Application.h"
#include "Titan/Core/Input.h"
#include "Titan/Core/KeyCodes.h"
#include "Titan/Core/Log.h"
#include "Titan/PCH.h"
#include "Titan/Renderer/Renderer.h"

namespace Titan
{

    Application* Application::s_Instance = nullptr;

    Application::Application()
    {
        TI_PROFILE_BEGIN_SESSION("Startup", "profile-startup.json");
        TI_CORE_ASSERT(!s_Instance, "Application already exists! There can only be one");
        s_Instance = this;

        m_Window = std::unique_ptr<Window>(Window::Create());
        m_Window->SetEventCallback(TI_BIND_EVENT_FN(Application::OnEvent));

        Renderer::Init();

        m_ImGuiLayer = new ImGuiLayer();
        PushOverlay(m_ImGuiLayer);
        TI_PROFILE_END_SESSION();
    }

    Application::~Application() {}

    void Application::Run()
    {
        TI_PROFILE_BEGIN_SESSION("Runtime", "profile-runtime.json");

        while (m_Running)
        {
            TI_PROFILE_SCOPE("Application::Run Gameloop");

            float time = (float)glfwGetTime(); // TODO: Platform Indepentend Time Query (Time::GetCurrent()???)
            Timestep timestep = time - m_LastFrameTime;
            m_LastFrameTime = time;
            if (!m_Minimized)
            {
                TI_PROFILE_SCOPE("Application::Run layer->OnUpdate");
                for (Layer* layer : m_LayerStack)
                    layer->OnUpdate(timestep);
            }

            m_ImGuiLayer->Begin();
            for (Layer* layer : m_LayerStack)
            {
                TI_PROFILE_SCOPE("Application::Run layer->OnImGuiRender");
                layer->OnImGuiRender(ImGui::GetCurrentContext());
            }
            m_ImGuiLayer->End();

            m_Window->OnUpdate();

            if (Input::IsKeyPressed(TI_KEY_ESCAPE))
                m_Running = false;
        }
        TI_PROFILE_END_SESSION();
    }

    void Application::OnEvent(Event& e)
    {
        TI_PROFILE_FUNCTION();

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
        TI_PROFILE_FUNCTION();

        m_LayerStack.PushLayer(layer);
    }

    void Application::PushOverlay(Layer* layer)
    {
        TI_PROFILE_FUNCTION();

        m_LayerStack.PushOverlay(layer);
    }

    bool Application::OnWindowResize(WindowResizeEvent& e)
    {
        TI_PROFILE_FUNCTION();

        if (e.GetWidth() == 0 || e.GetHeight() == 0)
        {
            m_Minimized = true;
            return false;
        }

        m_Minimized = false;
        Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

        return false;
    }

    bool Application::OnWindowClosed(WindowCloseEvent& e)
    {
        TI_PROFILE_FUNCTION();

        m_Running = false;
        return true;
    }

    void TI_API Titan::DeleteApplication(Application* app)
    {
        TI_PROFILE_BEGIN_SESSION("Shutdown", "profile-shutdown.json");
        delete app;
        TI_PROFILE_END_SESSION();
    }

} // namespace Titan