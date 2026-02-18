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
        static std::filesystem::path GetAppDataDirectory();
        static std::filesystem::path GetExecutablePath();
        static std::filesystem::path GetExecutableDirectory();
    };

    class TI_API Platform
    {
    public:
        static std::string GetPlatform();
        static std::string GetArchitecture();
        static std::string GetVariant();
    };

    class TI_API Process
    {
    public:
        static size_t GetCurrentProcessMemoryUsage();
        static size_t GetCurrentProcessVirtualMemoryUsage();
        static void RestartWithSameArgs();
    };

} // namespace Titan