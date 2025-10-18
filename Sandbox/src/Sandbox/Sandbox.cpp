#include <TerraEngine.h>
#include <stdio.h>

class Sandbox : public Terra::Application
{
public:
    Sandbox()
    {
        printf("Hello from Sandbox.exe\n");
    }
    ~Sandbox()
    {
    }
};

Terra::Application *Terra::CreateApplication()
{
    return new Sandbox();
}