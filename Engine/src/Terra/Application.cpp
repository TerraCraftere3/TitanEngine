#include "Application.h"

#include "Log.h"
#include "Events/ApplicationEvent.h"

#include <stdio.h>

namespace Terra
{

    Application::Application() {}

    Application::~Application() {}

    void Application::Run()
    {
        WindowResizeEvent e(1280, 720);
        if (e.IsInCategory(EventCategoryApplication))
        {
            LOG_TRACE(e.ToString());
        }
    }

} // namespace Terra