#pragma once

#include "PCH.h"
#include "Application.h"
#include "Log.h"

#ifdef TERRA_PLATFORM_WINDOWS

extern Terra::Application *Terra::CreateApplication();

int main(int argc, char **argv)
{
    Terra::Log::Init();
    LOG_CORE_INFO("Initialized Logger");
    Terra::Application *app = Terra::CreateApplication();
    LOG_CORE_INFO("Initialized Application");
    app->Run();
    delete app;
    LOG_CORE_INFO("Shutdown Application");
}

#endif