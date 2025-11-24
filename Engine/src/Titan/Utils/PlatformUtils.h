#pragma once

#include "Titan/PCH.h"

namespace Titan
{

    class TI_API FileDialogs
    {
    public:
        static std::string OpenFile(const char* filter);
        static std::string SaveFile(const char* filter);
    };

    class TI_API Debug
    {
    public:
        static bool isRenderdocAttached();
    };

    class TI_API Filesystem
    {
    public:
        static std::string GetAppDataDirectory();
        static std::string GetExecutablePath();
        static std::string GetExecutableDirectory();
    };

} // namespace Titan