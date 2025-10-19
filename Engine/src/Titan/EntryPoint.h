#pragma once

#include "Titan/Application.h"
#include "Titan/Log.h"
#include "Titan/PCH.h"

#ifdef TI_PLATFORM_WINDOWS

extern Titan::Application* Titan::CreateApplication();

int main(int argc, char** argv)
{
    Titan::Log::Init();
    LOG_CORE_INFO("Initialized Logger");
    Titan::Application* app = Titan::CreateApplication();
    LOG_CORE_INFO("Initialized Application");
    app->Run();
    delete app;
    LOG_CORE_INFO("Shutdown Application");
}

#endif