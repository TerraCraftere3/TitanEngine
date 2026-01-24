#include <Titan/Core/Application.h>
#include <Titan/Core/EntryPoint.h>
#include "SandboxLayer.h"

namespace Titan
{
    class SandboxApp : public Titan::Application
    {
    public:
        SandboxApp(const ApplicationSpecification& spec) : Application(spec)
        {
            PushLayer(new SandboxLayer());
            Application::GetInstance()->GetWindow().Maximize();
        }
        ~SandboxApp() {}
    };
} // namespace Titan

Titan::Application* Titan::CreateApplication(ApplicationCommandLineArgs args)
{
    ApplicationSpecification spec;
    spec.Name = "Sandbox";
    spec.WindowProperties = WindowProps("Sandbox");
    spec.CommandLineArgs = args;
    spec.Theme = UITheme::Dark;
    spec.DisableImGui = true;

    return new Titan::SandboxApp(spec);
}