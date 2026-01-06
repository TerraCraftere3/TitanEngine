---
sidebar_position: 2
---

# Physics

## Concepts & Usage

### Creating 3D Physics Bodies
```cpp
#include <Titan/Scene/Entity.h>
#include <Titan/Scene/Components.h>

// Create a dynamic rigidbody
Entity rigidbody = scene->CreateEntity("DynamicObject");
auto& rb = rigidbody.AddComponent<RigidbodyComponent>();
rb.Type = RigidbodyComponent::BodyType::Dynamic;
rb.FixedRotation = false;

// Create a static rigidbody
Entity staticBody = scene->CreateEntity("Ground");
auto& staticRb = staticBody.AddComponent<RigidbodyComponent>();
staticRb.Type = RigidbodyComponent::BodyType::Static;

// Kinematic rigidbody (moved by code, not physics)
Entity kinematicBody = scene->CreateEntity("Platform");
auto& kinRb = kinematicBody.AddComponent<RigidbodyComponent>();
kinRb.Type = RigidbodyComponent::BodyType::Kinematic;
```

### Creating 3D Colliders
```cpp
#include <Titan/Scene/PhysicsMaterial.h>

// Create a cube collider
Entity cubeObject = scene->CreateEntity("Cube");
auto& cubeCollider = cubeObject.AddComponent<CubeColliderComponent>();
cubeCollider.Offset = glm::vec3(0.0f, 0.0f, 0.0f);
cubeCollider.Size = glm::vec3(1.0f, 1.0f, 1.0f);

// Configure physics material
cubeCollider.Material->Density = 1.0f;
cubeCollider.Material->Friction = 0.5f;
cubeCollider.Material->Restitution = 0.2f;

// Create a sphere collider
Entity sphereObject = scene->CreateEntity("Sphere");
auto& sphereCollider = sphereObject.AddComponent<SphereColliderComponent>();
sphereCollider.Offset = glm::vec3(0.0f, 0.0f, 0.0f);
sphereCollider.Radius = 0.5f;
sphereCollider.Material->Friction = 0.8f;
```

### Creating 2D Physics Bodies
```cpp
// Create a 2D dynamic rigidbody
Entity rb2d = scene->CreateEntity("2DObject");
auto& rb2dComp = rb2d.AddComponent<Rigidbody2DComponent>();
rb2dComp.Type = Rigidbody2DComponent::BodyType::Dynamic;
rb2dComp.FixedRotation = true;

// Create a 2D box collider
auto& boxCollider = rb2d.AddComponent<BoxCollider2DComponent>();
boxCollider.Offset = glm::vec2(0.0f, 0.0f);
boxCollider.Size = glm::vec2(1.0f, 1.0f);
boxCollider.Material->Density = 1.0f;
boxCollider.Material->Friction = 0.5f;

// Create a 2D circle collider
Entity circlePhys = scene->CreateEntity("2DCircle");
circlePhys.AddComponent<Rigidbody2DComponent>().Type = Rigidbody2DComponent::BodyType::Dynamic;
auto& circleCollider = circlePhys.AddComponent<CircleCollider2DComponent>();
circleCollider.Radius = 0.5f;
circleCollider.Material->Restitution = 0.8f;
```

### Physics Materials
```cpp
// Modify physics material properties
Ref<PhysicsMaterial> material = CreateRef<PhysicsMaterial>();
material->Density = 2.0f;           // Higher density = heavier
material->Friction = 0.7f;           // More friction = slower sliding
material->Restitution = 0.4f;        // Bounciness (0 = no bounce, 1 = perfect bounce)
material->RestitutionThreshold = 0.5f; // Minimum impact speed for bounce
```
---

## Capabilities
The Physics Engine has support for the following shapes:
- [x] 2D Circle Collider
- [x] 2D Quad Collider
- [ ] Mesh Collider
- [x] Cube Collider
- [x] Sphere Collider

Also each Collider / Rigidbody has a physics material with options like Density, Friction, Restitution and Restitution Threshold:
| Property              | 2D  | 3D  |
| --------------------- | --- | --- |
| Density               | ✅   | ❌   |
| Friction              | ✅   | ✅   |
| Restitution           | ✅   | ✅   |
| Restitution Threshold | ✅   | ❌   |
---

## Backend
The Physics Simulation uses [PhysX](https://github.com/NVIDIA-Omniverse/PhysX) for 3D and [Box2D](github.com/erincatto/box2d) for 2D Physics Simulation. PhysX provides Industry standard Physics Simulation.
