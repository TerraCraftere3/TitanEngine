#include "Titan/Input.h"
#include "Titan/PCH.h"

namespace Titan
{

    class WindowsInput : public Input
    {
    protected:
        virtual bool IsKeyPressedImpl(int keycode) override;
        virtual bool IsMouseButtonPressedImpl(int button) override;
        virtual float GetMouseXImpl() override;
        virtual float GetMouseYImpl() override;
    };

} // namespace Titan