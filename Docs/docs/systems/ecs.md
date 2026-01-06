---
sidebar_position: 3
title: ECS
---

# Entity Component System
## Overview
The ECS uses [entt](https://github.com/skypjack/entt), which is an extremly fast entity component system without abstractions like child, parent relations, etc.
That means the engine has an extra components (`RelationshipComponent` in `Engine\src\Titan\Scene\Components.h`)

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