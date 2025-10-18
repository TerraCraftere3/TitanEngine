#include "ImGuiLayer.h"

// clang-format off
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>
#include <imgui.h>
// clang-format on

#include "Application.h"
#include "PCH.h"

namespace Titan
{

    ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer") {}

    ImGuiLayer::~ImGuiLayer() {}

    void ImGuiLayer::OnAttach()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::StyleColorsDark();

        ImGuiIO& io = ImGui::GetIO();
        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

        ImGui_ImplOpenGL3_Init("#version 410");
    }

    void ImGuiLayer::OnDetach() {}

    void ImGuiLayer::OnUpdate()
    {
        // Prepare Frame
        auto instance = Application::GetInstance();

        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(instance->GetWindow().GetWidth(), instance->GetWindow().GetHeight());

        float time = (float)glfwGetTime();
        io.DeltaTime = m_Time > 0.0 ? (time - m_Time) : (1.0f / 60.0f);
        m_Time = time;
        // Start Frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        // Render Frame
        static bool show = true;
        ImGui::ShowDemoWindow(&show);

        // End Frame
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void ImGuiLayer::OnEvent(Event& event) {}

} // namespace Titan