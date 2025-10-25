#include "Titan/ImGuiLayer.h"
#include "Titan/Core/Application.h"
#include "Titan/PCH.h"

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

namespace Titan
{

    ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer") {}

    ImGuiLayer::~ImGuiLayer() {}

    void ImGuiLayer::OnAttach()
    {
        TI_PROFILE_FUNCTION();

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows

        ImGui::StyleColorsDark();

        SetupStyles();

        Application* app = Application::GetInstance();
        GLFWwindow* window = static_cast<GLFWwindow*>(app->GetWindow().GetNativeWindow());

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 410");
    }

    void ImGuiLayer::OnDetach()
    {
        TI_PROFILE_FUNCTION();

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiLayer::Begin()
    {
        TI_PROFILE_FUNCTION();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void ImGuiLayer::End()
    {
        TI_PROFILE_FUNCTION();

        ImGuiIO& io = ImGui::GetIO();
        Application* app = Application::GetInstance();
        io.DisplaySize = ImVec2(app->GetWindow().GetWidth(), app->GetWindow().GetHeight());

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }

    void ImGuiLayer::SetupStyles()
    {
        ImGuiIO& io = ImGui::GetIO();

#ifdef TI_PLATFORM_WINDOWS
        // Try to load a system font (Segoe UI) — modern and crisp
        io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
#else
        // Fallback to the default ImGui font
        io.Fonts->AddFontDefault();
#endif

        ImGuiStyle& style = ImGui::GetStyle();

        // ======= Layout & Sizing =======
        style.WindowPadding = ImVec2(8, 8);
        style.FramePadding = ImVec2(6, 4);
        style.ItemSpacing = ImVec2(8, 6);
        style.ScrollbarSize = 14.0f;
        style.WindowBorderSize = 0.0f;
        style.FrameBorderSize = 0.0f;
        style.TabBorderSize = 0.0f;

        // ======= Rounding =======
        style.WindowRounding = 8.0f;
        style.FrameRounding = 5.0f;
        style.GrabRounding = 4.0f;
        style.TabRounding = 4.0f;

        // ======= Colors =======
        ImVec4* colors = style.Colors;

        colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.13f, 1.00f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.14f, 0.14f, 0.15f, 1.00f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.16f, 0.16f, 0.17f, 0.98f);

        colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.22f, 0.60f);
        colors[ImGuiCol_FrameBg] = ImVec4(0.18f, 0.18f, 0.19f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.25f, 0.27f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.35f, 0.35f, 0.38f, 1.00f);

        colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.08f, 0.08f, 0.10f, 0.75f);

        colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.27f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.35f, 0.35f, 0.38f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.40f, 0.40f, 0.42f, 1.00f);

        colors[ImGuiCol_Header] = ImVec4(0.25f, 0.25f, 0.28f, 1.00f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.35f, 0.35f, 0.38f, 1.00f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.40f, 0.40f, 0.45f, 1.00f);

        colors[ImGuiCol_Separator] = ImVec4(0.23f, 0.23f, 0.25f, 0.6f);
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.26f, 0.28f, 0.25f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.38f, 0.38f, 0.40f, 0.45f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.45f, 0.45f, 0.47f, 0.65f);

        colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.35f, 0.35f, 0.40f, 1.00f);
        colors[ImGuiCol_TabActive] = ImVec4(0.25f, 0.25f, 0.30f, 1.00f);

        colors[ImGuiCol_CheckMark] = ImVec4(0.35f, 0.60f, 1.00f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.35f, 0.60f, 1.00f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.50f, 0.70f, 1.00f, 1.00f);

        colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.45f, 0.90f, 0.35f);

        colors[ImGuiCol_NavHighlight] = ImVec4(0.25f, 0.45f, 0.90f, 0.70f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.30f, 0.50f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.10f, 0.10f, 0.15f, 0.60f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.10f, 0.10f, 0.15f, 0.60f);
    }

} // namespace Titan