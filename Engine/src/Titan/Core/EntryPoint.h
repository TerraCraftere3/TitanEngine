#pragma once

#include "Titan/Core/Application.h"
#include "Titan/Core/Log.h"
#include "Titan/PCH.h"

#ifdef TI_PLATFORM_WINDOWS

extern Titan::Application* Titan::CreateApplication();

int main(int argc, char** argv)
{
    Titan::Log::Init();

    Titan::Application* app = Titan::CreateApplication();
    TI_CORE_INFO("Initialized Application");

    app->Run();

    delete app;
    TI_CORE_INFO("Shutdown Application");
}

#endif