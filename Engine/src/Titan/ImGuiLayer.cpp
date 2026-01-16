#include "Titan/ImGuiLayer.h"
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

#include <ImGuizmo.h>
#include <backends/imgui_impl_glfw.h>
#include <imgui.h>
#include <implot.h>
#include "ImGuiLayer.h"
#include "Renderer/ImGuiBackend.h"

namespace Titan
{

    ImGuiLayer::ImGuiLayer(UITheme theme) : Layer("ImGuiLayer"), m_Theme(theme) {}

    ImGuiLayer::~ImGuiLayer() {}

    void ImGuiLayer::OnAttach()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImPlot::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows

        std::filesystem::path configPath = std::filesystem::path(Filesystem::GetAppDataDirectory()) / "Layout.ini";
        TI_CORE_TRACE("ImGui config path: {0}", configPath.string());
        try
        {
            std::filesystem::create_directories(configPath.parent_path());
        }
        catch (...)
        {
        }
        m_ImGuiConfigPath = configPath.string();
        io.IniFilename = m_ImGuiConfigPath.c_str();

        ImGui::StyleColorsDark();

        SetTheme(m_Theme);

        Application* app = Application::GetInstance();
        GLFWwindow* window = static_cast<GLFWwindow*>(app->GetWindow().GetNativeWindow());

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplTitan_Init();
    }

    void ImGuiLayer::OnDetach()
    {
        ImGui_ImplTitan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
    }

    void ImGuiLayer::Begin()
    {
        ImGui_ImplTitan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();
    }

    void ImGuiLayer::End()
    {
        ImGuiIO& io = ImGui::GetIO();
        Application* app = Application::GetInstance();
        io.DisplaySize = ImVec2(app->GetWindow().GetWidth(), app->GetWindow().GetHeight());

        ImGui::Render();
        ImGui_ImplTitan_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }

    void ImGuiLayer::SetTheme(UITheme theme)
    {
        if (theme == UITheme::Dark)
            SetupDarkStyles();
        else if (theme == UITheme::OLED)
            SetupOLEDStyles();
    }

    void ImGuiLayer::SetupOLEDStyles()
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();
        const float fontSize = 18.0f;

#ifdef TI_PLATFORM_WINDOWS
        io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", fontSize);
#else
        io.Fonts->AddFontDefault();
#endif

        // Icon font (FontAwesome)
        static const ImWchar iconRanges[] = {0xf000, 0xf3ff, 0};
        ImFontConfig config;
        config.MergeMode = true;
        config.PixelSnapH = true;
        io.Fonts->AddFontFromFileTTF("resources/fonts/fa-solid-900.ttf", fontSize, &config, iconRanges);

        // ===== Layout & Spacing =====
        style.WindowPadding = ImVec2(10, 10);
        style.FramePadding = ImVec2(6, 4);
        style.ItemSpacing = ImVec2(6, 4);
        style.ItemInnerSpacing = ImVec2(4, 2);
        style.IndentSpacing = 16.0f;
        style.ScrollbarSize = 12.0f;
        style.GrabMinSize = 10.0f;

        // ===== Rounding =====
        style.WindowRounding = 2.0f;
        style.ChildRounding = 2.0f;
        style.FrameRounding = 3.0f;
        style.PopupRounding = 3.0f;
        style.GrabRounding = 2.0f;
        style.TabRounding = 3.0f;

        // ===== Borders =====
        style.WindowBorderSize = 0.0f;
        style.ChildBorderSize = 1.0f;
        style.FrameBorderSize = 0.0f;
        style.PopupBorderSize = 1.0f;
        style.TabBorderSize = 0.0f;

        // ===== Colors =====
        ImVec4* colors = style.Colors;

        // Base
        colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.05f, 0.05f, 0.05f, 1.0f); // very dark gray
        colors[ImGuiCol_ChildBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.95f); // slightly lighter
        colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.95f);

        // Borders & separators
        colors[ImGuiCol_Border] = ImVec4(0.2f, 0.2f, 0.25f, 0.8f);
        colors[ImGuiCol_Separator] = ImVec4(0.2f, 0.2f, 0.25f, 0.8f);

        // Frames & buttons
        colors[ImGuiCol_FrameBg] = ImVec4(0.1f, 0.1f, 0.12f, 1.0f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.15f, 0.15f, 0.2f, 1.0f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.2f, 0.2f, 0.25f, 1.0f);

        colors[ImGuiCol_Button] = ImVec4(0.12f, 0.12f, 0.15f, 1.0f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.2f, 0.2f, 0.3f, 1.0f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.3f, 0.3f, 0.4f, 1.0f);

        // Title bars
        colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.1f, 1.0f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.0f, 0.5f, 0.8f, 1.0f); // strong cyan accent
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.7f);

        // Tabs
        colors[ImGuiCol_Tab] = ImVec4(0.08f, 0.08f, 0.1f, 1.0f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.0f, 0.45f, 0.85f, 1.0f);
        colors[ImGuiCol_TabActive] = ImVec4(0.0f, 0.5f, 0.9f, 1.0f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.02f, 0.02f, 0.02f, 1.0f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.05f, 0.05f, 0.05f, 1.0f);

        // Headers / collapsing
        colors[ImGuiCol_Header] = ImVec4(0.0f, 0.55f, 0.95f, 0.3f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.0f, 0.55f, 0.95f, 0.6f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.0f, 0.7f, 1.0f, 0.9f);

        // Highlights / sliders / checkmarks
        colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 0.8f, 1.0f, 1.0f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.0f, 0.8f, 1.0f, 1.0f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.9f, 1.0f, 1.0f);

        // Scrollbars
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.25f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.15f, 0.15f, 0.15f, 0.8f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.25f, 0.25f, 0.25f, 0.8f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.3f, 0.3f, 0.3f, 0.9f);

        // ImPlot / plotting
        colors[ImGuiCol_PlotLines] = ImVec4(0.0f, 0.8f, 1.0f, 1.0f);
        colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.0f, 0.9f, 1.0f, 1.0f);
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.0f, 0.8f, 1.0f, 1.0f);
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.0f, 0.9f, 1.0f, 1.0f);

        // Text selection
        colors[ImGuiCol_TextSelectedBg] = ImVec4(0.0f, 0.5f, 0.8f, 0.4f);
    }

    void ImGuiLayer::SetupDarkStyles()
    {
        ImGuiIO& io = ImGui::GetIO();
        const float fontSize = 18.0f;

#ifdef TI_PLATFORM_WINDOWS
        io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", fontSize);
#else
        io.Fonts->AddFontDefault();
#endif

        // Icon font (FontAwesome)
        static const ImWchar iconRanges[] = {0xf000, 0xf3ff, 0};
        ImFontConfig config;
        config.MergeMode = true;
        config.PixelSnapH = true;
        io.Fonts->AddFontFromFileTTF("resources/fonts/fa-solid-900.ttf", fontSize, &config, iconRanges);

        ImGuiStyle& style = ImGui::GetStyle();

        // ===== Layout & Spacing =====
        style.WindowPadding = ImVec2(12, 12);
        style.FramePadding = ImVec2(8, 5);
        style.ItemSpacing = ImVec2(8, 6);
        style.ItemInnerSpacing = ImVec2(6, 4);
        style.TouchExtraPadding = ImVec2(0, 0);
        style.IndentSpacing = 18.0f;
        style.ColumnsMinSpacing = 6.0f;
        style.ScrollbarSize = 14.0f;
        style.GrabMinSize = 10.0f;
        style.ColorButtonPosition = ImGuiDir_Right;
        style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
        style.SelectableTextAlign = ImVec2(0.0f, 0.0f);
        style.DisplayWindowPadding = ImVec2(19, 19);
        style.DisplaySafeAreaPadding = ImVec2(3, 3);
        style.MouseCursorScale = 1.0f;
        style.AntiAliasedLines = true;
        style.AntiAliasedLinesUseTex = true;
        style.AntiAliasedFill = true;
        style.CurveTessellationTol = 1.25f;
        style.CircleTessellationMaxError = 0.3f;

        // ===== Rounding =====
        style.WindowRounding = 8.0f;
        style.ChildRounding = 6.0f;
        style.FrameRounding = 5.0f;
        style.PopupRounding = 4.0f;
        style.GrabRounding = 4.0f;
        style.LogSliderDeadzone = 4.0f;
        style.TabRounding = 5.0f;

        // ===== Borders =====
        style.WindowBorderSize = 0.0f;
        style.ChildBorderSize = 1.0f;
        style.FrameBorderSize = 0.0f;
        style.PopupBorderSize = 1.0f;
        style.TabBorderSize = 1.0f;

        // ===== Colors =====
        ImVec4* colors = style.Colors;

        // Base
        colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
        colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.13f, 1.00f);
        colors[ImGuiCol_ChildBg] = ImVec4(0.14f, 0.14f, 0.15f, 1.00f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.16f, 0.16f, 0.17f, 0.95f);

        // Borders & separators
        colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.22f, 0.65f);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_Separator] = ImVec4(0.23f, 0.23f, 0.25f, 0.75f);
        colors[ImGuiCol_SeparatorHovered] = ImVec4(0.35f, 0.35f, 0.38f, 0.78f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(0.35f, 0.60f, 1.00f, 1.00f);

        // Frames
        colors[ImGuiCol_FrameBg] = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.28f, 0.28f, 0.30f, 1.00f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.35f, 0.35f, 0.38f, 1.00f);

        // Title bars
        colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.08f, 0.08f, 0.10f, 0.75f);

        // Tabs
        colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.35f, 0.35f, 0.40f, 1.00f);
        colors[ImGuiCol_TabActive] = ImVec4(0.25f, 0.25f, 0.30f, 1.00f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);

        // Buttons
        colors[ImGuiCol_Button] = ImVec4(0.25f, 0.25f, 0.28f, 1.00f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.38f, 0.38f, 0.42f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.50f, 0.50f, 0.55f, 1.00f);

        // Headers (collapsing sections)
        colors[ImGuiCol_Header] = ImVec4(0.25f, 0.25f, 0.28f, 1.00f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.35f, 0.35f, 0.38f, 1.00f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.45f, 0.45f, 0.50f, 1.00f);

        // Slider & checkmark
        colors[ImGuiCol_CheckMark] = ImVec4(0.35f, 0.60f, 1.00f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.35f, 0.60f, 1.00f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.50f, 0.70f, 1.00f, 1.00f);

        // Scrollbars
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.12f, 0.12f, 0.12f, 0.50f);
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30f, 0.30f, 0.33f, 0.80f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.45f, 0.80f);
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.50f, 0.50f, 0.55f, 0.90f);

        // Navigation / Selection
        colors[ImGuiCol_NavHighlight] = ImVec4(0.25f, 0.45f, 0.90f, 0.70f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.30f, 0.50f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.10f, 0.10f, 0.15f, 0.60f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.10f, 0.10f, 0.15f, 0.60f);

        // Tables
        colors[ImGuiCol_TableHeaderBg] = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
        colors[ImGuiCol_TableBorderStrong] = ImVec4(0.35f, 0.35f, 0.40f, 1.00f);
        colors[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.30f, 1.00f);
        colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.05f, 0.05f, 0.05f, 0.1f);

        // Drag & drop
        colors[ImGuiCol_DragDropTarget] = ImVec4(0.25f, 0.50f, 0.90f, 0.90f);

        // Plotting (implot)
        colors[ImGuiCol_PlotLines] = ImVec4(0.35f, 0.60f, 1.00f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.50f, 0.70f, 1.00f, 1.00f);
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.35f, 0.60f, 1.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.50f, 0.70f, 1.00f, 1.00f);

        // Text selection
        colors[ImGuiCol_TextSelectedBg] = ImVec4(0.35f, 0.60f, 1.00f, 0.35f);

        // Drag & drop
        colors[ImGuiCol_DragDropTarget] = ImVec4(0.25f, 0.50f, 0.90f, 0.90f);

        // Modal windows
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.70f);
    }

} // namespace Titan