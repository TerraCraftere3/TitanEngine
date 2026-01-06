---
sidebar_position: 2
---

# Physics

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
