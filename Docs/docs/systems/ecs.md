---
sidebar_position: 3
title: ECS
---

# Entity Component System
## Overview
The ECS uses [entt](https://github.com/skypjack/entt), which is an extremly fast entity component system without abstractions like child, parent relations, etc.
That means the engine has an extra components (`RelationshipComponent` in `Engine\src\Titan\Scene\Components.h`)

---

## Concepts & Usage

### Creating Entities
```cpp
#include <Titan/Scene/Entity.h>
#include <Titan/Scene/Components.h>

// Create an entity with a tag
Entity entity = scene->CreateEntity("MyEntity");

// Create entity from an entity ID (if you have the ID)
Entity existingEntity(entityID, scene.get());
```

### Adding Components
```cpp
// Add a transform component (automatically added to all entities)
auto& transform = entity.GetComponent<TransformComponent>();
transform.Translation = glm::vec3(5.0f, 2.0f, 0.0f);
transform.Rotation = glm::vec3(0.0f, glm::radians(45.0f), 0.0f);
transform.Scale = glm::vec3(1.0f, 1.0f, 1.0f);

// Add a mesh renderer
auto& meshRenderer = entity.AddComponent<MeshRendererComponent>();
meshRenderer.MeshRef = Mesh::Create("cube");

// Add a camera
auto& camera = entity.AddComponent<CameraComponent>();
camera.Primary = true;

// Add a rigidbody
auto& rb = entity.AddComponent<RigidbodyComponent>();
rb.Type = RigidbodyComponent::BodyType::Dynamic;

// Add a cube collider
auto& collider = entity.AddComponent<CubeColliderComponent>();
collider.Size = glm::vec3(1.0f, 1.0f, 1.0f);
```

### Accessing Components
```cpp
// Get component (asserts if component doesn't exist)
auto& transform = entity.GetComponent<TransformComponent>();

// Check if entity has component
if (entity.HasComponent<MeshRendererComponent>())
{
    auto& meshRenderer = entity.GetComponent<MeshRendererComponent>();
    // Use component...
}
```

### Removing Components
```cpp
// Remove a component
entity.RemoveComponent<CameraComponent>();
```

### Component Examples

#### Sprite Renderer (2D)
```cpp
auto& sprite = entity.AddComponent<SpriteRendererComponent>();
sprite.Tex = Assets::Load<Texture2D>("assets/textures/player.png");
sprite.Color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // Red with full alpha
```

#### Circle Renderer (2D)
```cpp
auto& circle = entity.AddComponent<CircleRendererComponent>();
circle.Color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
circle.Thickness = 0.1f;
circle.Fade = 0.01f;
```

#### Directional Light
```cpp
auto& light = entity.AddComponent<DirectionalLightComponent>();
light.Direction = glm::normalize(glm::vec3(0.5f, -1.0f, 0.5f));
```

#### Skybox
```cpp
auto& skybox = entity.AddComponent<SkyboxComponent>();
skybox.mode = SkyboxComponent::Mode::Colorramp;
skybox.colorrampSettings.TopColor = glm::vec3(0.1f, 0.3f, 0.7f);
skybox.colorrampSettings.BottomColor = glm::vec3(0.9f, 0.9f, 1.0f);

// Or use HDRI
// skybox.mode = SkyboxComponent::Mode::HDRI;
// skybox.hdriSettings.Skybox = Assets::Load<Cubemap>("assets/hdri/sky.hdr");
```

#### Post Processing
```cpp
auto& postFX = entity.AddComponent<PostFXComponent>();
postFX.TonemappingSettings.isEnabled = true;
postFX.TonemappingSettings.Operator = TonemappingOperator::Filmic;
postFX.TonemappingSettings.Exposure = 1.0f;
postFX.FXAASettings.isEnabled = true;
```

#### Script
```cpp
auto& script = entity.AddComponent<ScriptComponent>();
script.ClassName = "GameScripts.PlayerController";
```

### Destroying Entities
```cpp
scene->DestroyEntity(entity);
```

### Parent-Child Relationships
```cpp
// Set up hierarchy
Entity parent = scene->CreateEntity("Parent");
Entity child = scene->CreateEntity("Child");

auto& parentRel = parent.GetComponent<RelationshipComponent>();
auto& childRel = child.GetComponent<RelationshipComponent>();

childRel.Parent = parent;
parentRel.Children.push_back(child);

// Child's transform is now relative to parent
```

### Iterating Entities
```cpp
// Get all entities with a specific component
auto view = scene->GetAllEntitiesWith<TransformComponent, MeshRendererComponent>();
for (auto entityID : view)
{
    Entity entity(entityID, scene.get());
    auto& transform = entity.GetComponent<TransformComponent>();
    auto& mesh = entity.GetComponent<MeshRendererComponent>();
    // Process entity...
}
```

---

## Components
### Basic
- ID Component (`IDComponent`): Contains the UUID of the Entity
- Tag Component (`TagComponent`): Contains the Tag of the Entity
- Transform Component (`TransformComponent`): Contains the Translation, Rotation and Scale of the Entity and manages World -> Local Transformations
- Relationship Component (`RelationshipComponent`): Contains the Parent and List of Children of the Entity

### 2D Renderer
- Sprite Renderer Component (`SpriteRendererComponent`): Renders a 2D sprite with texture and color tint
- Circle Renderer Component (`CircleRendererComponent`): Renders a 2D circle with customizable thickness and fade

### 3D Renderer
- Mesh Renderer Component (`MeshRendererComponent`): Renders a 3D mesh
- Directional Light Component (`DirectionalLightComponent`): Defines a directional light source with direction
- Skybox Component (`SkyboxComponent`): Renders the scene background with either HDRI or color ramp modes

### 2D Physics
- Rigidbody 2D Component (`Rigidbody2DComponent`): Controls 2D physics simulation (Static, Dynamic, or Kinematic body types)
- Box Collider 2D Component (`BoxCollider2DComponent`): Defines a rectangular 2D collision shape with offset and size
- Circle Collider 2D Component (`CircleCollider2DComponent`): Defines a circular 2D collision shape with offset and radius

### 3D Physics
- Rigidbody Component (`RigidbodyComponent`): Controls 3D physics simulation (Static, Dynamic, or Kinematic body types)
- Cube Collider Component (`CubeColliderComponent`): Defines a cubic 3D collision shape with offset and size
- Sphere Collider Component (`SphereColliderComponent`): Defines a spherical 3D collision shape with offset and radius

### Other
- Camera Component (`CameraComponent`): Defines a camera with projection settings and primary camera flag
- Post FX Component (`PostFXComponent`): Applies post-processing effects (FXAA, Tonemapping)
- Script Component (`ScriptComponent`): Attaches a C# script class to an entity
- LookAt Component (`LookAtComponent`): Defines a target position for camera or entity look-at behavior

:::info
All Components are located in `Engine\src\Titan\Scene\Components.h` and can be easily modified. E.g. when adding, add them to the `AllComponents` struct and `SceneSerializer.h`
:::