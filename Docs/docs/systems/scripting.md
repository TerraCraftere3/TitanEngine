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
    }
}
```

---

## Available Callbacks

| Callback | When Called |
|----------|-----------|
| `OnCreate()` | When entity enters play mode |
| `OnUpdate(float deltaTime)` | Every frame during play |

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
