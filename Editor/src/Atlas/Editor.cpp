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
    spec.Name = "Atlas Editor";
    spec.CommandLineArgs = args;

    return new Titan::AtlasApp(spec);
}