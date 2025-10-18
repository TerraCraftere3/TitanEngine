#pragma once

#include "Core.h"
#include "Events/ApplicationEvent.h"
#include "Events/Event.h"
#include "Terra/PCH.h"
#include "Window.h"

namespace Terra
{
    class TERRA_API Application
    {
    public:
        Application();
        virtual ~Application();

        void Run();
        void OnEvent(Event& e);

    private:
        bool OnWindowClosed(WindowCloseEvent event);

    private:
        std::unique_ptr<Window> m_Window;
        bool m_Running = true;
    };

    // Declared by Client
    Application* CreateApplication();

} // namespace Terra