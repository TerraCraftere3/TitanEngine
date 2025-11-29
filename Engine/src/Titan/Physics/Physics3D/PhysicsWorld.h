#pragma once

#include "Physics3D.h"
#include "Titan/Core/Timestep.h"
#include "Titan/PCH.h"
#include "Titan/Scene/Components.h"

namespace Titan::Physics3D
{
    class PhysicsWorld
    {
    public:
        PhysicsWorld(const glm::vec3& gravity = {0.0f, -9.81f, 0.0f});
        ~PhysicsWorld();

        void Step(Timestep ts);

        // Returns a pointer to the created Px actor (user should treat as opaque void*)
        void* CreateBody(RigidbodyComponent& rb, TransformComponent& transform,
                         const CubeColliderComponent* cube = nullptr, const SphereColliderComponent* sphere = nullptr);
        void DestroyBody(void* actor);

        // Read back actor/global pose. Returns true if successful.
        bool GetBodyTransform(void* actor, glm::vec3& outPosition, glm::vec3& outEulerAngles);

    private:
        class Impl;
        Impl* m_Impl = nullptr;
    };

} // namespace Titan::Physics3D
