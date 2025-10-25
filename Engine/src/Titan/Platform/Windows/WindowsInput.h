#include "Titan/Core/Input.h"
#include "Titan/PCH.h"

namespace Titan
{

    class WindowsInput : public Input
    {
    protected:
        virtual bool IsKeyPressedImpl(KeyCode keycode) override;
        virtual bool IsMouseButtonPressedImpl(MouseButton button) override;
        virtual float GetMouseXImpl() override;
        virtual float GetMouseYImpl() override;
    };

} // namespace Titan