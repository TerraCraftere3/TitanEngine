#pragma once

#include "Titan/Core/Application.h"
#include "Titan/Core/Log.h"
#include "Titan/PCH.h"

#ifdef TI_PLATFORM_WINDOWS

extern Titan::Application* Titan::CreateApplication(ApplicationCommandLineArgs args);

int main(int argc, char** argv)
{
    Titan::Log::Init();

    Titan::Application* app = Titan::CreateApplication({argc, argv});
    TI_CORE_INFO("Initialized Application");

    app->Run();

    Titan::DeleteApplication(app);
    TI_CORE_INFO("Shutdown Application");
}

#endif