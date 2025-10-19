#include "Titan/Application.h"

#include "Titan/Input.h"
#include "Titan/KeyCodes.h"
#include "Titan/Log.h"

namespace Titan
{

    Application* Application::s_Instance = nullptr;

    static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
    {
        switch (type)
        {
            case Titan::ShaderDataType::Float:
                return GL_FLOAT;
            case Titan::ShaderDataType::Float2:
                return GL_FLOAT;
            case Titan::ShaderDataType::Float3:
                return GL_FLOAT;
            case Titan::ShaderDataType::Float4:
                return GL_FLOAT;
            case Titan::ShaderDataType::Mat3:
                return GL_FLOAT;
            case Titan::ShaderDataType::Mat4:
                return GL_FLOAT;
            case Titan::ShaderDataType::Int:
                return GL_INT;
            case Titan::ShaderDataType::Int2:
                return GL_INT;
            case Titan::ShaderDataType::Int3:
                return GL_INT;
            case Titan::ShaderDataType::Int4:
                return GL_INT;
            case Titan::ShaderDataType::Bool:
                return GL_BOOL;
        }

        TI_CORE_ASSERT(false, "Unknown ShaderDataType!");
        return 0;
    }

    Application::Application()
    {
        TI_CORE_ASSERT(!s_Instance, "Application already exists! There can only be one");
        s_Instance = this;

        m_Window = std::unique_ptr<Window>(Window::Create());
        m_Window->SetEventCallback(TI_BIND_EVENT_FN(Application::OnEvent));

        m_ImGuiLayer = new ImGuiLayer();
        PushOverlay(m_ImGuiLayer);
    }

    Application::~Application() {}

    void Application::Run()
    {
        while (m_Running)
        {
            for (Layer* layer : m_LayerStack)
                layer->OnUpdate();

            m_ImGuiLayer->Begin();
            for (Layer* layer : m_LayerStack)
                layer->OnImGuiRender(ImGui::GetCurrentContext());
            m_ImGuiLayer->End();

            m_Window->OnUpdate();

            if (Input::IsKeyPressed(TI_KEY_ESCAPE))
                m_Running = false;
        }
    }

    void Application::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(TI_BIND_EVENT_FN(Application::OnWindowClosed));

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

    bool Application::OnWindowClosed(WindowCloseEvent event)
    {
        m_Running = false;
        return true;
    }

} // namespace Titan