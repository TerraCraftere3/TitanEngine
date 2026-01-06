---
sidebar_position: 1
---

# Renderer

## Concepts & Usage

### Creating Meshes
```cpp
#include <Titan/Renderer/Mesh.h>

// Create built-in primitive meshes
Ref<Mesh> cubeMesh = Mesh::Create("cube");
Ref<Mesh> sphereMesh = Mesh::Create("sphere");
Ref<Mesh> planeMesh = Mesh::Create("plane");

// Load mesh from file
Ref<Mesh> customMesh = Assets::Load<Mesh>("assets/models/character.fbx");
```

### Creating Textures
```cpp
#include <Titan/Renderer/Texture.h>

// Load a 2D texture
Ref<Texture2D> diffuseTexture = Assets::Load<Texture2D>("assets/textures/diffuse.png");
Ref<Texture2D> normalTexture = Assets::Load<Texture2D>("assets/textures/normal.png");

// Create empty texture (for rendering targets)
Texture2D::Specification spec{};
spec.Width = 1920;
spec.Height = 1080;
spec.Format = ImageFormat::RGBA8;
Ref<Texture2D> renderTarget = Texture2D::Create(spec);
```

### Creating Framebuffers
```cpp
#include <Titan/Renderer/Framebuffer.h>

// Create a framebuffer with attachments
FramebufferSpecification fbSpec{};
fbSpec.Width = 1920;
fbSpec.Height = 1080;
fbSpec.Attachments.Attachments = {
    { ImageFormat::RGBA8 },  // Color attachment
    { ImageFormat::Depth24Stencil8 }  // Depth attachment
};
Ref<Framebuffer> framebuffer = Framebuffer::Create(fbSpec);

// Bind for rendering
framebuffer->Bind();
RenderCommand::Clear();
// ... render ...
framebuffer->Unbind();

// Read pixels from framebuffer
uint32_t pixelData = framebuffer->ReadPixel(0, mouseX, mouseY);
```

### Vertex Buffers and Arrays
```cpp
#include <Titan/Renderer/VertexArray.h>
#include <Titan/Renderer/Buffer.h>

// Create vertex data
float vertices[] = {
    -0.5f, -0.5f, 0.0f,  // Position
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
};

// Create vertex buffer
Ref<VertexBuffer> vertexBuffer = VertexBuffer::Create(vertices, sizeof(vertices));

// Define vertex layout
BufferLayout layout = {
    { ShaderDataType::Float3, "Position" }
};
vertexBuffer->SetLayout(layout);

// Create index buffer
uint32_t indices[] = { 0, 1, 2 };
Ref<IndexBuffer> indexBuffer = IndexBuffer::Create(indices, 3);

// Create vertex array and add buffers
Ref<VertexArray> vertexArray = VertexArray::Create();
vertexArray->AddVertexBuffer(vertexBuffer);
vertexArray->SetIndexBuffer(indexBuffer);

// Render the vertex array
RenderCommand::DrawIndexed(vertexArray);
```

### Materials and Shaders
```cpp
#include <Titan/Renderer/Material.h>
#include <Titan/Renderer/Shader.h>

// Load a shader
Ref<Shader> shader = Assets::Load<Shader>("assets/shaders/pbr.glsl");

// Create a material
Ref<Material> material = CreateRef<Material>(shader);

// Set material properties
material->SetFloat("Roughness", 0.5f);
material->SetFloat("Metallic", 0.8f);
material->SetVector3("Albedo", glm::vec3(1.0f, 0.0f, 0.0f));
material->SetTexture("AlbedoMap", diffuseTexture);
material->SetTexture("NormalMap", normalTexture);
```

### Rendering
```cpp
#include <Titan/Renderer/Renderer2D.h>
#include <Titan/Renderer/SceneRenderer.h>

// 2D Rendering
Renderer2D::BeginScene(camera);
Renderer2D::DrawQuad(position, size, color);
Renderer2D::DrawRotatedQuad(position, size, rotation, texture, color);
Renderer2D::EndScene();

// 3D Scene Rendering (deferred)
SceneRenderer::RenderSceneEditor(viewportIndex, scene, camera, overlaySettings);
SceneRenderer::RenderSceneRuntime(scene);
```

---

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
OpenGL has support for most features that are not hardware dependant, for example Tesselation Shaders, but no support for complex stuff like **Hardware** Raytracing
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
### DirectX 11 and 12
**For now no DX11 or DX12 Support**
### Metal
**For now no Metal Support**

---
In The Future this is all going to change (new apis, features)

:::info
All Rendering Abstractions are in `Engine\src\Titan\Renderer` and all api specific code is in `Engine\src\Titan\Platform\[API]`
:::