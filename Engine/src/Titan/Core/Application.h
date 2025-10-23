#pragma once

#include "Titan/Core.h"
#include "Titan/Core/LayerStack.h"
#include "Titan/Core/Timestep.h"
#include "Titan/Core/Window.h"
#include "Titan/Events/Event.h"
#include "Titan/ImGuiLayer.h"
#include "Titan/PCH.h"

namespace Titan
{
    class TI_API Application
    {
    public:
        Application(const std::string& name = "Titan App");
        virtual ~Application();

        void Close();
        void Run();
        void OnEvent(Event& e);

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* layer);

        inline Window& GetWindow() { return *m_Window; }
        inline static Application* GetInstance() { return s_Instance; };

    private:
        bool OnWindowClosed(WindowCloseEvent& e);
        bool OnWindowResize(WindowResizeEvent& e);

    private:
        static Application* s_Instance;
        std::unique_ptr<Window> m_Window;
        bool m_Running = true;
        bool m_Minimized = false;
        LayerStack m_LayerStack;
        ImGuiLayer* m_ImGuiLayer;
        float m_LastFrameTime = 0.0f;
    };

    void TI_API DeleteApplication(Application* app);

    // Declared by Client
    Application* CreateApplication();

} // namespace Titan