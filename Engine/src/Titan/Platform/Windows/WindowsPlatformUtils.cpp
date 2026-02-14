#include "Titan/Core/Application.h"
#include "Titan/PCH.h"
#include "Titan/Utils/PlatformUtils.h"
#include "Titan/Utils/String.h"

// clang-format off
#ifdef APIENTRY
    #undef APIENTRY
#endif
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <shlobj.h>
#include <windows.h>
#include <psapi.h>
#include <string>

namespace Titan
{

    std::string FileDialogs::OpenFile(const char* filter)
    {
        OPENFILENAMEA ofn;
        CHAR szFile[260] = {0};
        CHAR currentDir[256] = {0};
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::GetInstance()->GetWindow().GetNativeWindow());
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        if (GetCurrentDirectoryA(256, currentDir))
            ofn.lpstrInitialDir = currentDir;
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
        if (GetOpenFileNameA(&ofn) == TRUE)
        {
            return ofn.lpstrFile;
        }
        return std::string();
    }

    std::string FileDialogs::SaveFile(const char* filter)
    {
        OPENFILENAMEA ofn;
        CHAR szFile[260] = {0};
        CHAR currentDir[256] = {0};
        ZeroMemory(&ofn, sizeof(OPENFILENAME));
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::GetInstance()->GetWindow().GetNativeWindow());
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile);
        if (GetCurrentDirectoryA(256, currentDir))
            ofn.lpstrInitialDir = currentDir;
        ofn.lpstrFilter = filter;
        ofn.nFilterIndex = 1;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
        if (GetSaveFileNameA(&ofn) == TRUE)
        {
            return ofn.lpstrFile;
        }
        return std::string();
    }

    bool Debug::isRenderdocAttached()
    {
        return GetModuleHandleA("renderdoc.dll") != NULL;
    }

    std::filesystem::path Filesystem::GetAppDataDirectory()
    {
        CHAR path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path)))
        {
            return std::filesystem::path(std::string(path) + "\\" +
                                         SanitizeName(Application::GetInstance()->GetSpecification().Name));
        }
        return "";
    }

    std::filesystem::path Filesystem::GetExecutablePath()
    {
        CHAR path[MAX_PATH];
        GetModuleFileNameA(NULL, path, MAX_PATH);
        return std::filesystem::path(path);
    }

    std::filesystem::path Filesystem::GetExecutableDirectory()
    {
        std::filesystem::path exePath = GetExecutablePath();
        size_t pos = exePath.string().find_last_of("\\/");
        if (pos != std::string::npos)
        {
            return std::filesystem::path(exePath.string().substr(0, pos));
        }
        return "";
    }

    size_t Process::GetCurrentProcessMemoryUsage()
    {
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
        {
            return pmc.WorkingSetSize;
        }
        return 0;
    }

    size_t Process::GetCurrentProcessVirtualMemoryUsage()
    {
        PROCESS_MEMORY_COUNTERS pmc;
        if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
        {
            return pmc.PagefileUsage;
        }
        return 0;
    }

} // namespace Titan