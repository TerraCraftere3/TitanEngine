---
sidebar_position: 4
title: Input
---

# Input System

## Overview
The Input System handles keyboard, mouse, and other input device interactions. It provides a simple polling-based API to check the state of input devices during the update cycle.

---

## Input Handling

### Keyboard Input
Query keyboard state using key codes defined in `KeyCodes.h`:

```cpp
#include <Titan/Core/Input.h>

// Check if a key is currently pressed
if (Input::IsKeyPressed(KeyCode::W))
{
    // Handle W key press
}

// Supported modifiers
if (Input::IsKeyPressed(KeyCode::LeftControl))
{
    // Handle Ctrl key
}
```

### Mouse Input
Handle mouse position and button states:

```cpp
// Get mouse position
auto [mouseX, mouseY] = Input::GetMousePosition();

// Check mouse button state
if (Input::IsMouseButtonPressed(MouseButton::ButtonLeft))
{
    // Handle left mouse button
}

if (Input::IsMouseButtonPressed(MouseButton::ButtonRight))
{
    // Handle right mouse button
}
```

---

## Key Codes
All available key codes are defined in `Engine\src\Titan\Core\KeyCodes.h`:
- **Alphanumeric**: `A`-`Z`, `0`-`9`
- **Function Keys**: `F1`-`F12`
- **Navigation**: `Up`, `Down`, `Left`, `Right`, `Home`, `End`, `PageUp`, `PageDown`
- **Special Keys**: `Escape`, `Tab`, `Enter`, `Backspace`, `Delete`, `Space`
- **Modifiers**: `LeftControl`, `RightControl`, `LeftShift`, `RightShift`, `LeftAlt`, `RightAlt`

---

## Mouse Buttons
Available mouse buttons defined in `Engine\src\Titan\Core\MouseButtons.h`:
- `ButtonLeft` - Left mouse button
- `ButtonRight` - Right mouse button
- `ButtonMiddle` - Middle mouse button (wheel)

---

## Events
For more advanced input handling with event callbacks, the engine uses an event system. Input events are dispatched automatically and can be handled via event listeners:

- `KeyPressedEvent` - Triggered when a key is pressed
- `KeyReleasedEvent` - Triggered when a key is released
- `MouseButtonPressedEvent` - Triggered when a mouse button is pressed
- `MouseButtonReleasedEvent` - Triggered when a mouse button is released
- `MouseMovedEvent` - Triggered when the mouse moves

See the [Events documentation](/docs/systems/events) for more details on event handling.

:::info
Input is located in `Engine\src\Titan\Core\Input.h` and the platform-specific implementations are in `Engine\src\Titan\Platform\[Platform]\*Input.cpp`
:::
