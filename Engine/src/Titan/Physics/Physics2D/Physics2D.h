#pragma once

#include "Titan/PCH.h"
#include "Titan/Core/Timestep.h"
#include "Titan/Scene/Components.h"

namespace Titan::Physics2D
{
    class PhysicsWorld
    {
    public:
        PhysicsWorld(const glm::vec2& gravity = {0.0f, -9.8f});
        ~PhysicsWorld();

        void Step(Timestep ts, int32_t velocityIterations = 6, int32_t positionIterations = 2);

        // Creates a physics body for the given Rigidbody2DComponent and transform.
        // Returns an opaque pointer to the backend body (stored in RuntimeBody).
        void* CreateBody(const Rigidbody2DComponent& rb2d, const TransformComponent& transform,
                         const BoxCollider2DComponent* box, const CircleCollider2DComponent* circle);

        glm::vec2 GetBodyPosition(void* body) const;
        float GetBodyAngle(void* body) const;

    private:
        struct Impl;
        Impl* m_Impl = nullptr;
    };

} // namespace Titan::Physics2D
