#pragma once

#include "Titan/Core.h"
#include "Titan/Events/ApplicationEvent.h"
#include "Titan/Events/Event.h"
#include "Titan/ImGuiLayer.h"
#include "Titan/Core/LayerStack.h"
#include "Titan/PCH.h"
#include "Titan/Renderer/Buffer.h"
#include "Titan/Renderer/Camera.h"
#include "Titan/Renderer/Renderer.h"
#include "Titan/Renderer/Shader.h"
#include "Titan/Renderer/VertexArray.h"
#include "Titan/Core/Window.h"

namespace Titan
{
    class TI_API Application
    {
    public:
        Application();
        virtual ~Application();

        void Run();
        void OnEvent(Event& e);

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* layer);

        inline Window& GetWindow() { return *m_Window; }
        inline static Application* GetInstance() { return s_Instance; };

    private:
        bool OnWindowClosed(WindowCloseEvent event);

    private:
        static Application* s_Instance;
        std::unique_ptr<Window> m_Window;
        bool m_Running = true;
        LayerStack m_LayerStack;
        ImGuiLayer* m_ImGuiLayer;
    };

    // Declared by Client
    Application* CreateApplication();

} // namespace Titan