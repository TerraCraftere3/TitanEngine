#include "Physics2D.h"
#include "box2d/box2d.h"

namespace Titan::Physics2D
{
    struct PhysicsWorld::Impl
    {
        Impl(const glm::vec2& gravity)
        {
            world = new b2World(b2Vec2(gravity.x, gravity.y));
        }

        ~Impl()
        {
            delete world;
            world = nullptr;
        }

        b2World* world = nullptr;
    };

    static b2BodyType Rigidbody2DTypeToBox2DBody(Rigidbody2DComponent::BodyType bodyType)
    {
        switch (bodyType)
        {
        case Rigidbody2DComponent::BodyType::Static:
            return b2_staticBody;
        case Rigidbody2DComponent::BodyType::Dynamic:
            return b2_dynamicBody;
        case Rigidbody2DComponent::BodyType::Kinematic:
            return b2_kinematicBody;
        }

        TI_CORE_ASSERT(false, "Unknown body type");
        return b2_staticBody;
    }

    PhysicsWorld::PhysicsWorld(const glm::vec2& gravity)
    {
        m_Impl = new Impl(gravity);
    }

    PhysicsWorld::~PhysicsWorld()
    {
        delete m_Impl;
        m_Impl = nullptr;
    }

    void PhysicsWorld::Step(Timestep ts, int32_t velocityIterations, int32_t positionIterations)
    {
        if (!m_Impl || !m_Impl->world)
            return;

        m_Impl->world->Step((float)ts, velocityIterations, positionIterations);
    }

    void* PhysicsWorld::CreateBody(const Rigidbody2DComponent& rb2d, const TransformComponent& transform,
                                   const BoxCollider2DComponent* box, const CircleCollider2DComponent* circle)
    {
        if (!m_Impl || !m_Impl->world)
            return nullptr;

        b2BodyDef bodyDef;
        bodyDef.type = Rigidbody2DTypeToBox2DBody(rb2d.Type);
        bodyDef.position.Set(transform.Translation.x, transform.Translation.y);
        bodyDef.angle = transform.Rotation.z;

        b2Body* body = m_Impl->world->CreateBody(&bodyDef);
        body->SetFixedRotation(rb2d.FixedRotation);

        if (box)
        {
            auto mat = box->Material;

            b2PolygonShape boxShape;
            boxShape.SetAsBox(box->Size.x * transform.Scale.x, box->Size.y * transform.Scale.y);

            b2FixtureDef fixtureDef;
            fixtureDef.shape = &boxShape;
            fixtureDef.density = mat->Density;
            fixtureDef.friction = mat->Friction;
            fixtureDef.restitution = mat->Restitution;
            fixtureDef.restitutionThreshold = mat->RestitutionThreshold;
            body->CreateFixture(&fixtureDef);
        }

        if (circle)
        {
            auto mat = circle->Material;

            b2CircleShape circleShape;
            circleShape.m_p.Set(circle->Offset.x, circle->Offset.y);
            circleShape.m_radius = transform.Scale.x * circle->Radius;

            b2FixtureDef fixtureDef;
            fixtureDef.shape = &circleShape;
            fixtureDef.density = mat->Density;
            fixtureDef.friction = mat->Friction;
            fixtureDef.restitution = mat->Restitution;
            fixtureDef.restitutionThreshold = mat->RestitutionThreshold;
            body->CreateFixture(&fixtureDef);
        }

        return (void*)body;
    }

    glm::vec2 PhysicsWorld::GetBodyPosition(void* body) const
    {
        if (!body)
            return glm::vec2(0.0f);

        b2Body* b = (b2Body*)body;
        b2Vec2 p = b->GetPosition();
        return glm::vec2(p.x, p.y);
    }

    float PhysicsWorld::GetBodyAngle(void* body) const
    {
        if (!body)
            return 0.0f;

        b2Body* b = (b2Body*)body;
        return b->GetAngle();
    }

} // namespace Titan::Physics2D
