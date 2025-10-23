# Titan Engine [![CMake on Windows (with submodules, CMake >= 4.0)](https://github.com/TerraCraftere3/TitanEngine/actions/workflows/cmake_windows.yml/badge.svg)](https://github.com/TerraCraftere3/TitanEngine/actions/workflows/cmake_windows.yml)

**A Remake of my old Engine**

## 📝 Folder Structure
- `Engine` contains the code for the Engine that is used by the Game and the Editor
  - `src/Titan` contains the engine code
    - `Core` contains the core code used by all Systems
    - `Events` contains all code for the Event System
    - `Platform` contains platform specific code (os and graphics api specific)
    - `Renderer` contains platform independent code for the Renderer
- `Sandbox` contains the code of the Game
- `Vendor` contains all Dependencies of `Engine` (and `Sandbox`)

## 📦 Dependencies

| Dependency | Usage                 | Link                             |
| ---------- | --------------------- | -------------------------------- |
| spdlog     | Logging               | https://github.com/gabime/spdlog |
| glfw       | Windowing / Input     | https://github.com/glfw/glfw     |
| glad       | OpenGL Loader         | https://glad.dav1d.de/           |
| imgui      | GUI                   | https://github.com/ocornut/imgui |
| glm        | Math / Linear Algebra | https://github.com/g-truc/glm    |
| stb_image  | Texture Loader        | https://github.com/nothings/stb  |

## ⚙️ Features
- Platform Independent Code
- 2D Batch Renderer

## 🧪 Builds
| Tool  | Operating System | Badge                                                                                                                                                                                                                                 |
| ----- | ---------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| CMake | Windows          | [![CMake on Windows (with submodules, CMake >= 4.0)](https://github.com/TerraCraftere3/TitanEngine/actions/workflows/cmake_windows.yml/badge.svg)](https://github.com/TerraCraftere3/TitanEngine/actions/workflows/cmake_windows.yml) |

(only supports Windows for now)



## 🖌️ Styleguide

### 1. File Structure

* **Header files (`.h` / `.hpp`)**

  * Use `#pragma once` as the header guard.
  * Keep includes minimal: include only what is necessary.

* **Source files (`.cpp`)**

  * Include the corresponding header first.
  * Then include standard library headers and third-party headers.
  * Implementation of functions and methods goes inside the matching namespace.

### 2. Namespaces

* Wrap all code in the project namespace, e.g., `Titan`.
* Close namespaces explicitly with comments:

```cpp
} // namespace Titan
```

### 3. Classes

* Class names use **PascalCase**.
* Member variables:

  * Prefix with `m_`
  * Use PascalCase after `m_`
* Public, protected, and private sections:

  * Group logically; declare `public` first if it is the primary interface.
  * Use multiple `private:` sections if needed for clarity (optional).
* Virtual destructor for base classes.

```cpp
class Application
{
public:
    Application();
    virtual ~Application();

    void Run();

public:
    int PublicInt;

private:
    void PrivateFunction();

private:
    int m_Member;
};
```

### 4. Functions / Methods

* **Naming:** PascalCase.
* **Implementation:**

  * Scope function definitions within the namespace.
  * Use the fully qualified name in `.cpp` files:

```cpp
void Application::Run()
{
}
```

* Always use braces for consistency, even on single-line blocks.

### 5. Variables

* Member variables: `m_MemberName`
* Static Member variables: `s_MemberName`
* Local variables: `camelCase` or `snake_case` (choose one and be consistent)
* Constants: `ALL_CAPS` or `kPascalCase` for constants (choose a consistent style)

### 6. Includes

* Order:

  1. Corresponding header file
  2. Standard library headers
  3. Third-party library headers
  4. Project headers

* The Order should only be changed if dependencies break (e.g. glfw and glad)

```cpp
#include "Application.h"
#include <stdio.h>
#include <SomeLib/SomeHeader.h>
#include "Core.h"
```

* Prefer forward declarations in headers when possible to reduce compilation dependencies.

### 7. Formatting

* Indentation: **4 spaces** per level, **no tabs**.
* Braces:

  * Opening brace on the same line for classes, functions, and control statements.
* One blank line between functions.
* Maximum line length: 120 characters.

```cpp
Application::Application()
{
}

Application::~Application()
{
}
```

### 8. Miscellaneous

* Logging, macros, and utility functions should follow the same naming conventions.
* Comments:

  * Use `//` for single-line comments.
  * Explain “why”, not “what” when commenting.
* Avoid `using namespace` in headers.

## 📜 License
GNU General Public License V3