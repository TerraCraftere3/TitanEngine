#include "Titan/Core/Application.h"
#include "Titan/Core/Input.h"
#include "Titan/Core/KeyCodes.h"
#include "Titan/Core/Log.h"
#include "Titan/PCH.h"
#include "Titan/Renderer/Renderer.h"
#include "Titan/Scripting/ScriptEngine.h"
// clang-format off
#ifdef APIENTRY
    #undef APIENTRY
#endif
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on
namespace Titan
{

    Application* Application::s_Instance = nullptr;

    Application::Application(const std::string& name)
    {
        TI_PROFILE_BEGIN_SESSION("Startup", "profile-startup");
        TI_CORE_ASSERT(!s_Instance, "Application already exists! There can only be one");
        s_Instance = this;

        m_Window = Scope<Window>(Window::Create(WindowProps(name)));
        m_Window->SetEventCallback(TI_BIND_EVENT_FN(Application::OnEvent));

        Renderer::Init();
        ScriptEngine::Init();

        m_ImGuiLayer = new ImGuiLayer();
        PushOverlay(m_ImGuiLayer);
        TI_PROFILE_END_SESSION();
    }

    Application::~Application()
    {
        TI_PROFILE_FUNCTION();

        ScriptEngine::Shutdown();
    }

    void Application::Close()
    {
        TI_PROFILE_FUNCTION();
        m_Running = false;
    }

    void Application::Run()
    {
        TI_PROFILE_BEGIN_SESSION("Runtime", "profile-runtime");

        while (m_Running)
        {
            TI_PROFILE_SCOPE("Application::Run Gameloop");

            float time = (float)glfwGetTime(); // TODO: Platform Indepentend Time Query (Time::GetCurrent()???)
            Timestep timestep = time - m_LastFrameTime;
            m_LastFrameTime = time;

            ExecuteMainThreadQueue();

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
        TI_PROFILE_FUNCTION();

        m_Running = false;
        return true;
    }

    void TI_API Titan::DeleteApplication(Application* app)
    {
        TI_PROFILE_BEGIN_SESSION("Shutdown", "profile-shutdown");
        delete app;
        TI_PROFILE_END_SESSION();
    }

} // namespace Titan