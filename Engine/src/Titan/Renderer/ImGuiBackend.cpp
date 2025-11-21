#include "ImGuiBackend.h"
#include "RenderCommand.h"
#include "RendererAPI.h"

#include <backends/imgui_impl_opengl3.h>

namespace Titan
{

    bool ImGui_ImplTitan_Init()
    {
        RendererAPI::API api = RendererAPI::GetAPI();
        switch (api)
        {
            case RendererAPI::API::OpenGL:
            {
                ImGui_ImplOpenGL3_Init("#version 440");
                return true;
            };
        }
        TI_CORE_ASSERT("ImGui could not initialize with unknown Renderer API!");
        return false;
    }

    void ImGui_ImplTitan_Shutdown()
    {
        RendererAPI::API api = RendererAPI::GetAPI();
        switch (api)
        {
            case RendererAPI::API::OpenGL:
            {
                ImGui_ImplOpenGL3_Shutdown();
                return;
            };
        }
        TI_CORE_ASSERT("ImGui could not initialize with unknown Renderer API!");
        return;
    }

    void ImGui_ImplTitan_NewFrame()
    {
        RendererAPI::API api = RendererAPI::GetAPI();
        switch (api)
        {
            case RendererAPI::API::OpenGL:
            {
                ImGui_ImplOpenGL3_NewFrame();
                return;
            };
        }
        TI_CORE_ASSERT("ImGui could not initialize with unknown Renderer API!");
    }

    void ImGui_ImplTitan_RenderDrawData(ImDrawData* draw_data)
    {
        RendererAPI::API api = RendererAPI::GetAPI();
        switch (api)
        {
            case RendererAPI::API::OpenGL:
            {
                ImGui_ImplOpenGL3_RenderDrawData(draw_data);
                return;
            };
        }
        TI_CORE_ASSERT("ImGui could not initialize with unknown Renderer API!");
    }

} // namespace Titan