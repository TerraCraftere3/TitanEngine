---
sidebar_position: 0
---

# Filestructure

This document explains the folder structure and modules of TitanEngine.

## ``Engine``
The core game engine library used by both the Editor and Sandbox (game runtime). Contains all fundamental systems.

### Engine Modules:
- **Engine/Core** - Foundation systems: Application lifecycle, Window management, Input handling, Logging, Layers, Timestep, UUID generation
- **Engine/Renderer** - Platform-independent rendering systems: PBR renderer, 2D renderer, Scene renderer, Skybox, Fullscreen rendering, Geometry renderer, Render graph, Shaders, Textures, Meshes, Materials, Buffers, Framebuffers, Cameras
- **Engine/Scene** - Entity Component System (ECS) and scene management: Scene serialization, Entity handling, Components, Scene camera, Physics materials, Asset management
- **Engine/Scripting** - C# scripting integration: Script engine, C# glue code for exposing engine functionality to scripts
- **Engine/Events** - Event system: Application events, Keyboard events, Mouse events
- **Engine/Platform** - Platform-specific implementations: OpenGL backend, Windows-specific code
- **Engine/Physics** - Physics simulation: 2D physics (Box2D), 3D physics (PhysX)
- **Engine/Debug** - Development tools: Profiling (Instrumentor), Debug macros
- **Engine/Utils** - Utility functions: Math helpers, AABB, Platform dialogs, String utilities

## ``Editor``
Atlas - the game editor application built on top of the Engine.

### Editor Modules:
- **Editor/Panels** - UI panels: Content Browser (asset management), Scene Hierarchy (entity tree view), Log Panel (console output)
- **Editor/Renderer** - Editor-specific rendering: Thumbnail generation for assets

## ``Script-Core``
C# library/API that game scripts use. Contains the managed (C#) side of the scripting interface with classes like Entity, Components, Input, Log, Vector types.

## ``Sandbox``
The game/runtime application - where you build your actual game using the Engine. Test bed for engine features.

## ``Runtime``
Output folder containing compiled executables and runtime dependencies (DLLs, assets, resources, mono runtime, e.g. the workspace folder).

## ``Vendor``
Third-party dependencies: assimp, Box2D, entt, filewatch, glad, glfw, glm, imgui, ImGuizmo, ImReflect, mono, nanosvg, optick, PhysX, slang, spdlog, stb, yaml-cpp.

## ``Scripts``
Build automation scripts: `Configure.bat` (runs CMake), `Build.bat` (compiles project).

## ``Docs``
Documentation website built with Docusaurus.
