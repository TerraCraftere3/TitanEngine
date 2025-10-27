# Titan Engine [![CMake on Windows (with submodules, CMake >= 4.0)](https://github.com/TerraCraftere3/TitanEngine/actions/workflows/cmake_windows.yml/badge.svg)](https://github.com/TerraCraftere3/TitanEngine/actions/workflows/cmake_windows.yml)

**A Remake of my old Engine**


## 🧰 How to Build
### Windows (x86_64)
1. Clone the repo using `git clone --recursive https://github.com/TerraCraftere3/TitanEngine`
2. Open the repo in the terminal (`cd TitanEngine`)
3. run `.\Scripts\Configure.bat` and `.\Scripts\Build.bat` in the terminal
4. Your done 🚀

### Linux
For now there is no linux support

## 📝 Folder Structure
- `Engine` contains the code for Titan Engine that is used by the Game and the Editor
  - `src/Titan` contains the engine code
    - `Core` contains the core code used by all Systems
    - `Events` contains all code for the Event System
    - `Platform` contains platform specific code (os and graphics api specific)
    - `Renderer` contains platform independent code for the Renderer
    - `Scene` contains code used by the ecs and scene renderer
    - `Debug` contains profiling and debugging code
    - `Utils` contains small utilities like save dialogs, etc
- `Editor` contains the code for Atlas (The Editor)
  - `src/Atlas` contains the editor code
    - `Panels` contains the code for each panel (asset viewer, etc.)
- `Sandbox` contains the code of the Game
- `Vendor` contains all Dependencies of `Engine`, `Editor` and `Sandbox`
- `Script` contains build scripts for the CMake Project

## 📦 Dependencies

| Name              | Usage / Purpose                                                 | Repository Link                                                             |
| ----------------- | --------------------------------------------------------------- | --------------------------------------------------------------------------- |
| **spdlog**        | Fast C++ logging library with support for formatting and sinks  | [gabime/spdlog](https://github.com/gabime/spdlog)                           |
| **glfw**          | Window, input, and context management for OpenGL/Vulkan         | [glfw/glfw](https://github.com/glfw/glfw)                                   |
| **imgui**         | Immediate-mode GUI library for tools and editors                | [ocornut/imgui](https://github.com/ocornut/imgui)                           |
| **glm**           | Mathematics library for graphics software (GLSL-style)          | [g-truc/glm](https://github.com/g-truc/glm)                                 |
| **stb**           | Single-file public domain libraries for image and asset loading | [nothings/stb](https://github.com/nothings/stb)                             |
| **entt**          | Fast and flexible ECS (Entity-Component-System) framework       | [skypjack/entt](https://github.com/skypjack/entt)                           |
| **yaml-cpp**      | YAML parser and emitter in C++                                  | [jbeder/yaml-cpp](https://github.com/jbeder/yaml-cpp)                       |
| **ImGuizmo**      | Gizmo controls for ImGui (translation, rotation, scaling)       | [CedricGuillemet/ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo)     |
| **SPIRV-Cross**   | SPIR-V reflection and cross-compilation library                 | [KhronosGroup/SPIRV-Cross](https://github.com/KhronosGroup/SPIRV-Cross)     |
| **SPIRV-Tools**   | Tools and libraries for processing SPIR-V modules               | [KhronosGroup/SPIRV-Tools](https://github.com/KhronosGroup/SPIRV-Tools)     |
| **SPIRV-Headers** | SPIR-V header files defining the instruction set                | [KhronosGroup/SPIRV-Headers](https://github.com/KhronosGroup/SPIRV-Headers) |
| **glslang**       | GLSL to SPIR-V reference compiler                               | [KhronosGroup/glslang](https://github.com/KhronosGroup/glslang)             |
| **shaderc**       | Shader compilation tools and libraries for Vulkan/GLSL          | [google/shaderc](https://github.com/google/shaderc)                         |
| **nanosvg** | SVG Software Rendering | [memononen/nanosvg](https://github.com/memononen/nanosvg) |


## ⚙️ Features
- Platform Independent Code
- 2D Batch Renderer
- Entity Component System
- Scene Renderer

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