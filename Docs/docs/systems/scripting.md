---
sidebar_position: 7
title: Scripting
---

# C# Scripting System

## Overview
The Scripting System enables C# script integration for game logic. Scripts are compiled to .NET assemblies and executed within the engine's runtime environment.

---

## Script Components

### Creating a Script Entity
```cpp
#include <Titan/Scene/Entity.h>
#include <Titan/Scene/Components.h>

Entity scriptEntity = scene->CreateEntity("ScriptedEntity");
auto& scriptComp = scriptEntity.AddComponent<ScriptComponent>();
scriptComp.ClassName = "MyNamespace.MyScript";
```

---

## C# Script Structure

### Basic Script Template
```csharp
using Titan;

namespace GameScripts
{
    public class PlayerController : Entity
    {
        private Vector3 moveDirection;

        public void OnCreate()
        {
            // Called when the entity is created in play mode
        }

        public void OnUpdate(float deltaTime)
        {
            // Called every frame during play mode
            HandleInput();
            Move(deltaTime);
        }

        public void OnPhysicsUpdate(float deltaTime)
        {
            // Called during physics simulation
        }

        public void OnDestroy()
        {
            // Called when the entity is destroyed
        }

        private void HandleInput()
        {
            // Handle input here
        }

        private void Move(float deltaTime)
        {
            // Movement logic here
        }
    }
}
```

---

## Available Callbacks

| Callback | When Called |
|----------|-----------|
| `OnCreate()` | When entity enters play mode |
| `OnUpdate(float deltaTime)` | Every frame during play |
| `OnPhysicsUpdate(float deltaTime)` | Every physics tick |
| `OnDestroy()` | When entity is destroyed or leaves play mode |
| `OnCollisionBegin(Entity other)` | When collision starts |
| `OnCollisionEnd(Entity other)` | When collision ends |

---

## Component Access

### Getting Components
```csharp
public class MyScript : Entity
{
    public void OnUpdate(float deltaTime)
    {
        // Get components from this entity
        var transform = GetComponent<TransformComponent>();
        var meshRenderer = GetComponent<MeshRendererComponent>();
        
        // Access component properties
        transform.Translation = new Vector3(0, 1, 0);
    }
}
```

### Adding Components
```csharp
public void OnCreate()
{
    // Add a component to this entity
    AddComponent<CameraComponent>();
    var camera = GetComponent<CameraComponent>();
}
```

---

## Accessing Other Entities

### Scene Queries
```csharp
public class MyScript : Entity
{
    public void OnUpdate(float deltaTime)
    {
        // Find entities by tag
        Entity player = Scene.FindEntityByTag("Player");
        
        // Access entities directly if you have a reference
        if (player)
        {
            var playerTransform = player.GetComponent<TransformComponent>();
        }
    }
}
```

---

## Physics Interactions

### Rigidbody Operations
```csharp
public class PhysicsScript : Entity
{
    public void OnCreate()
    {
        var rb = GetComponent<RigidbodyComponent>();
        // Configure rigidbody if needed
    }

    public void OnCollisionBegin(Entity other)
    {
        // Handle collision start
    }

    public void OnCollisionEnd(Entity other)
    {
        // Handle collision end
    }
}
```

---

## Assembly Management

### Reloading Scripts
```cpp
#include <Titan/Scripting/ScriptEngine.h>

// Hot-reload all C# scripts
ScriptEngine::ReloadAssembly();
```

The script system watches for changes to the Script-Core assembly and automatically reloads when modifications are detected during development.

---

## Debugging Scripts

The scripting system integrates with the engine's logging system:

```csharp
using Titan;

public class DebugScript : Entity
{
    public void OnUpdate(float deltaTime)
    {
        Log.Info("Script is running!");
        Log.Warn("This is a warning");
        Log.Error("This is an error");
    }
}
```

Logs appear in the engine's Log panel.

:::info
The Scripting System is located in `Engine\src\Titan\Scripting\` and uses Mono for .NET integration. C# source scripts are in the `Script-Core` folder.
:::
