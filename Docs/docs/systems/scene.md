---
sidebar_position: 6
title: Scene System
---

# Scene System

## Overview
The Scene System manages entities, their components, and the overall scene state. It handles initialization, updates, serialization, and provides the framework for both edit-time and runtime behavior.

---

## Scene States

The engine supports three main scene states:

| State | Description |
|-------|------------|
| **Edit** | Design-time mode where you can modify entities and components |
| **Play** | Runtime mode where physics, scripts, and game logic execute |
| **Simulate** | Physics simulation mode without script execution |

---

## Creating and Managing Scenes

### Creating a New Scene
```cpp
#include <Titan/Scene/Scene.h>

// Create an empty scene
Ref<Scene> scene = CreateRef<Scene>();

// Set viewport size
scene->OnViewportResize(1920, 1080);
```

### Creating Entities
```cpp
#include <Titan/Scene/Entity.h>

// Create an entity with a tag
Entity entity = scene->CreateEntity("MyEntity");

// Add components to the entity
auto& transform = entity.GetComponent<TransformComponent>();
transform.Translation = glm::vec3(0.0f, 1.0f, 0.0f);

entity.AddComponent<MeshRendererComponent>().MeshRef = Mesh::Create("cube");
```

### Destroying Entities
```cpp
scene->DestroyEntity(entity);
```

### Duplicating Entities
```cpp
Entity duplicate = scene->DuplicateEntity(entity);
```

---

## Scene Lifecycle

### Edit Mode
```cpp
// Called during each frame in edit mode
scene->OnUpdateEditor(timestep, editorCamera);

// Called when viewport is resized
scene->OnViewportResize(width, height);
```

### Play Mode
```cpp
// Initialize runtime state (called when entering play mode)
scene->OnRuntimeStart();

// Called each frame during playback
scene->OnUpdateRuntime(timestep);

// Cleanup runtime state (called when exiting play mode)
scene->OnRuntimeStop();
```

### Simulate Mode
```cpp
// Initialize simulation state
scene->OnSimulationStart();

// Called each frame during simulation
scene->OnUpdateSimulation(timestep, simulationCamera);

// Cleanup simulation state
scene->OnSimulationStop();
```

---

## Entity-Component Queries

### Iterating Entities with Components
```cpp
// Get all entities with a specific component
auto view = scene->GetAllEntitiesWith<TransformComponent>();
for (auto entity_id : view)
{
    Entity entity(entity_id, scene.get());
    auto& transform = entity.GetComponent<TransformComponent>();
    // Process entity
}

// Get entities with multiple components
auto view = scene->GetAllEntitiesWith<TransformComponent, MeshRendererComponent>();
```

---

## Scene Serialization

### Saving a Scene
```cpp
#include <Titan/Scene/SceneSerializer.h>

SceneSerializer serializer(scene);
serializer.Serialize("path/to/scene.titan");
```

### Loading a Scene
```cpp
Ref<Scene> loadedScene = Assets::Load<Scene>("path/to/scene.titan");
```

---

## Parent-Child Relationships

Entities can have parent-child relationships managed through the `RelationshipComponent`:

```cpp
// Add relationship component to establish hierarchy
Entity parent = scene->CreateEntity("Parent");
Entity child = scene->CreateEntity("Child");

auto& parentRel = parent.AddComponent<RelationshipComponent>();
auto& childRel = child.AddComponent<RelationshipComponent>();

childRel.Parent = parent;
parentRel.Children.push_back(child);

// Child transforms are now relative to parent
```

---

## Viewport Resizing

```cpp
// Handle viewport resize
scene->OnViewportResize(newWidth, newHeight);
```

This updates all cameras and rendering systems to match the new viewport dimensions.

:::info
The Scene System is located in `Engine\src\Titan\Scene\Scene.h` and related files. Entity management uses the entt library with Titan-specific extensions in `Entity.h` and `Entity.cpp`.
:::
