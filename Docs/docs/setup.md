---
sidebar_position: 1
---

# Setup Guide
## Windows
On Windows you need the following prequisits:
- Vulkan SDK (you can find it [here](https://vulkan.lunarg.com/))
- MSVC (easiest way to install is via the [Visual Studio Installer](https://visualstudio.microsoft.com/de/downloads/))

If you just installed them, restart your PC. Now that you are ready to clone and build the engine (enter the following commands in powershell or any other terminal):
1. `git clone --recursive https://github.com/TerraCraftere3/TitanEngine`
2. `cd TitanEngine`
3. `cmake -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE --no-warn-unused-cli -S . -B ./build`
4. `cmake --build ./build --config Release --target ALL_BUILD -j 12 --` (replace the -j 12 with how many cores you have, eg: ___-j [CORE_AMOUNT]___)

Now you are ready to use the Game Engine

## Linux & Mac
For now there is no Linux or Mac support (because i mainly develop on windows). There is probably going to be Linux support, allthough Mac is probably going to be left behind (because of the Hardware Requirements).
:::note
You can try to build it for Linux since there is nearly no inherit windows native code, except in `Engine/src/Titan/Platform/Windows/WindowsPlatformUtils.cpp`
:::