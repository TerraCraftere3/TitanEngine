---
sidebar_position: 1
---

# Renderer

## Capabilities
The Renderer has full PBR Support with features like Image Based Lighting. Here you have a Overview of the Features:
- [x] Deferred PBR 
- [x] Image Based Lighting
- [ ] Image Based Reflections
- [x] Tone Mapping
- [x] FXAA
- [ ] SSAO
- [ ] SSR
- [ ] Bloom

:::info
You can easilly add new Post Processing in `Engine\src\Titan\Renderer\SceneRenderer.cpp` and `Engine\src\Titan\Renderer\PostProcessing\*.cpp`, you dont need any specific graphics api knowledge (e.g. OpenGL, etc.)
:::
---

## Backends
### OpenGL
OpenGL has support for most features that are not hardware dependant, for example Tesselation Shaders.
| Version | Compute Shader | Tessellation Shader | Geometry Shader | Raytracing |
| ------- | -------------- | ------------------- | --------------- | ---------- |
| **4.6** | ✅              | ✅                   | ✅               | ❌          |
| **4.5** | ❌              | ✅                   | ✅               | ❌          |
| **4.4** | ❌              | ✅                   | ✅               | ❌          |
| **4.3** | ❌              | ✅                   | ✅               | ❌          |
| **4.2** | ❌              | ❌                   | ✅               | ❌          |
| **4.1** | ❌              | ❌                   | ✅               | ❌          |
| **4.0** | ❌              | ❌                   | ✅               | ❌          |
| **3.3** | ❌              | ❌                   | ✅               | ❌          |
| **3.2** | ❌              | ❌                   | ❌               | ❌          |

### Vulkan
**For now no Vulkan Support**
### DirectX 11
**For now no DX11 Support**
### DirectX 12
**For now no DX12 Support**
### Metal
**For now no Metal Support**

---
In The Future this is all going to change (new apis, features)