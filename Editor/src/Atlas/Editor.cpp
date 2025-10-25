#include <Titan.h>
#include <Titan/Core/EntryPoint.h>
#include "EditorLayer.h"

namespace Titan
{
    class AtlasApp : public Titan::Application
    {
    public:
        AtlasApp() : Application("Atlas Editor")
        {
            PushLayer(new EditorLayer());
            Application::GetInstance()->GetWindow().Maximize();
        }
        ~AtlasApp() {}
    };
} // namespace Titan

Titan::Application* Titan::CreateApplication()
{
    return new Titan::AtlasApp();
}