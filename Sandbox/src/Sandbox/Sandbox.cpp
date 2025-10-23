#include <Titan.h>
#include <Titan/Core/EntryPoint.h>
#include "Sandbox2D.h"

class Sandbox : public Titan::Application
{
public:
    Sandbox() : Application("Sandbox App") { PushLayer(new Sandbox2D()); }
    ~Sandbox() {}
};

Titan::Application* Titan::CreateApplication()
{
    return new Sandbox();
}