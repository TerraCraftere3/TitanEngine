---
sidebar_position: 5
title: Asset Management
---

# Asset Management System

## Overview
The Asset System provides a unified interface for loading, managing, and caching game assets like meshes, textures, materials, cubemaps, and scenes. Assets are loaded asynchronously and cached to avoid duplicate loading.

---

## Asset Types
The engine supports the following asset types:

| Type | Class | Extensions |
|------|-------|-----------|
| **Texture 2D** | `Texture2D` | `.png`, `.jpg`, `.hdr` |
| **Mesh** | `Mesh` | `.obj`, `.fbx`, `.gltf` |
| **Material** | `Material` | `.titan` |
| **Cubemap** | `Cubemap` | `.hdr`, `.ktx` |
| **Scene** | `Scene` | `.titan` |

---

## Loading Assets

### Basic Asset Loading
```cpp
#include <Titan/Scene/Assets.h>
#include <Titan/Renderer/Texture.h>
#include <Titan/Renderer/Mesh.h>

// Load a texture
Ref<Texture2D> texture = Assets::Load<Texture2D>("assets/textures/diffuse.png");

// Load a mesh
Ref<Mesh> mesh = Assets::Load<Mesh>("assets/models/character.fbx");

// Load a scene
Ref<Scene> scene = Assets::Load<Scene>("assets/scenes/Level1.titan");
```

### Built-in Primitives
```cpp
// Create built-in primitive meshes
Ref<Mesh> cubeMesh = Mesh::Create("cube");
Ref<Mesh> sphereMesh = Mesh::Create("sphere");
Ref<Mesh> planeMesh = Mesh::Create("plane");
```

---

## Asset Unloading

```cpp
// Unload a specific asset
Assets::Unload("assets/textures/diffuse.png");

// Unload by path
std::filesystem::path assetPath = "assets/scenes/Level1.titan";
Assets::Unload(assetPath);
```

---

## Asset Caching
Assets are automatically cached in memory after the first load. Subsequent loads return the cached reference, improving performance. Use `Unload()` to free cached assets when no longer needed.

---

## File Organization
Recommended asset folder structure:
```
assets/
├── textures/
│   ├── diffuse/
│   ├── normal/
│   └── pbr/
├── models/
├── materials/
├── scenes/
├── cubemaps/
└── icons/
```

---

## Asset Paths
Asset paths are relative to the engine's asset directory. The asset directory is typically located at the root of your project as `assets/`.

```cpp
// All paths are relative to the assets directory
Ref<Texture2D> tex = Assets::Load<Texture2D>("textures/diffuse.png");
// Loads from: <project>/assets/textures/diffuse.png
```

:::info
All Asset Management code is located in `Engine\src\Titan\Scene\Assets.h` and `Engine\src\Titan\Scene\Assets.cpp`. Loaders for specific formats are in `Engine\src\Titan\Scene\*Loader.cpp`
:::
