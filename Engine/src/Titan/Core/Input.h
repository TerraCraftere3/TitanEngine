#pragma once

#include "Titan/Core.h"
#include "Titan/Core/KeyCodes.h"
#include "Titan/Core/MouseButtons.h"
#include "Titan/PCH.h"

namespace Titan
{

    class TI_API Input
    {
    public:
        inline static bool IsKeyPressed(KeyCode keycode) { return s_Instance->IsKeyPressedImpl(keycode); }
        inline static bool IsMouseButtonPressed(MouseButton btn) { return s_Instance->IsMouseButtonPressedImpl(btn); }
        inline static float GetMouseX() { return s_Instance->GetMouseXImpl(); }
        inline static float GetMouseY() { return s_Instance->GetMouseYImpl(); }

    protected:
        virtual bool IsKeyPressedImpl(KeyCode keycode) = 0;
        virtual bool IsMouseButtonPressedImpl(MouseButton button) = 0;
        virtual float GetMouseXImpl() = 0;
        virtual float GetMouseYImpl() = 0;

    private:
        static Scope<Input> s_Instance;
    };

} // namespace Titan