#include <Titan/Core/Application.h>
#include <Titan/Core/EntryPoint.h>
#include "EditorLayer.h"

namespace Titan
{
    class AtlasApp : public Titan::Application
    {
    public:
        AtlasApp(const ApplicationSpecification& spec) : Application(spec)
        {
            PushLayer(new EditorLayer());
            Application::GetInstance()->GetWindow().Maximize();
        }
        ~AtlasApp() {}
    };
} // namespace Titan

Titan::Application* Titan::CreateApplication(ApplicationCommandLineArgs args)
{
    ApplicationSpecification spec;
    spec.Name = "AtlasEditor";
    spec.WindowProperties =
        WindowProps("Atlas Editor for Titan | " TI_PLATFORM_STRING " " TI_ARCH_STRING " (" TI_VARIANT_STRING ")");
    spec.CommandLineArgs = args;
    spec.Theme = UITheme::Dark;
    spec.DisableImGui = false;

    return new Titan::AtlasApp(spec);
}