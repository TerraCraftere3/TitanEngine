#pragma once

#include "Application.h"

#ifdef TERRA_PLATFORM_WINDOWS

extern Terra::Application *Terra::CreateApplication();

int main(int argc, char **argv)
{
    Terra::Application *app = Terra::CreateApplication();
    app->Run();
    delete app;
}

#endif