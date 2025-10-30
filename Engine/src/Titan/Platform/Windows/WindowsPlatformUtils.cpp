#include "Titan/Core/Application.h"
#include "Titan/PCH.h"
#include "Titan/Utils/PlatformUtils.h"

// clang-format off
#ifdef APIENTRY
    #undef APIENTRY
#endif
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

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

} // namespace Titan