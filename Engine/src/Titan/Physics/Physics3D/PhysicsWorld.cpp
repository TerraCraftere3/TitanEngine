#include "PhysicsWorld.h"
#include "Physics3D.h"
#include "PxPhysicsAPI.h"

using namespace physx;

namespace Titan::Physics3D
{
    struct PhysicsWorld::Impl
    {
        PxScene* Scene = nullptr;

        Impl() = default;
        ~Impl()
        {
            if (Scene)
            {
                Scene->release();
                Scene = nullptr;
            }
        }
    };

    static PxTransform ToPxTransform(const TransformComponent& t)
    {
        glm::quat q = glm::quat(t.Rotation);
        PxQuat pq(q.x, q.y, q.z, q.w);
        PxVec3 p(t.Translation.x, t.Translation.y, t.Translation.z);
        return PxTransform(p, pq);
    }

    PhysicsWorld::PhysicsWorld(const glm::vec3& gravity)
    {
        m_Impl = new Impl();

        PxPhysics* physics = GetPhysics();
        PxFoundation* foundation = GetFoundation();
        PxDefaultCpuDispatcher* dispatcher = GetDispatcher();

        if (!physics || !foundation)
        {
            TI_CORE_ERROR("PhysicsWorld created before PhysX initialization");
            return;
        }

        PxSceneDesc sceneDesc(physics->getTolerancesScale());
        sceneDesc.gravity = PxVec3(gravity.x, gravity.y, gravity.z);
        sceneDesc.cpuDispatcher = dispatcher ? dispatcher : PxDefaultCpuDispatcherCreate(2);
        sceneDesc.filterShader = PxDefaultSimulationFilterShader;

        m_Impl->Scene = physics->createScene(sceneDesc);
        if (!m_Impl->Scene)
        {
            TI_CORE_ERROR("Failed to create PhysX scene");
        }
    }

    PhysicsWorld::~PhysicsWorld()
    {
        delete m_Impl;
        m_Impl = nullptr;
    }

    void PhysicsWorld::Step(Timestep ts)
    {
        if (!m_Impl || !m_Impl->Scene)
            return;

        m_Impl->Scene->simulate(ts.GetSeconds());
        m_Impl->Scene->fetchResults(true);
    }

    void* PhysicsWorld::CreateBody(RigidbodyComponent& rb, TransformComponent& transform,
                                   const CubeColliderComponent* cube, const SphereColliderComponent* sphere)
    {
        if (!m_Impl || !m_Impl->Scene)
            return nullptr;

        PxPhysics* physics = GetPhysics();
        if (!physics)
            return nullptr;

        PxTransform pose = ToPxTransform(transform);

        PxRigidActor* actor = nullptr;
        if (rb.Type == RigidbodyComponent::BodyType::Static)
        {
            actor = physics->createRigidStatic(pose);
        }
        else
        {
            PxRigidDynamic* dyn = physics->createRigidDynamic(pose);
            if (rb.FixedRotation)
            {
                PxRigidDynamicLockFlags lockFlags = PxRigidDynamicLockFlag::eLOCK_ANGULAR_X |
                                                    PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y |
                                                    PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z;
                dyn->setRigidDynamicLockFlags(lockFlags);
            }
            actor = dyn;
        }

        if (!actor)
            return nullptr;

        if (cube)
        {
            glm::vec3 size = cube->Size * transform.Scale;

            PxBoxGeometry geom(size.x, size.y, size.z);

            float friction = 0.5f;
            float restitution = 0.1f;
            if (cube->Material)
            {
                friction = cube->Material->Friction;
                restitution = cube->Material->Restitution;
            }

            PxMaterial* pxMat = physics->createMaterial(friction, friction, restitution);

            PxShape* shape = physics->createShape(geom, *pxMat);
            if (shape)
            {
                actor->attachShape(*shape);
                shape->release();
            }

            pxMat->release();
        }
        if (sphere)
        {
            float scaledRadius = sphere->Radius * transform.Scale.x;

            PxSphereGeometry geom(scaledRadius);

            float friction = 0.5f;
            float restitution = 0.1f;
            if (sphere->Material)
            {
                friction = sphere->Material->Friction;
                restitution = sphere->Material->Restitution;
            }

            PxMaterial* pxMat = physics->createMaterial(friction, friction, restitution);

            PxShape* shape = physics->createShape(geom, *pxMat);
            if (shape)
            {
                actor->attachShape(*shape);
                shape->release();
            }

            pxMat->release();
        }

        m_Impl->Scene->addActor(*actor);

        return actor;
    }

    bool PhysicsWorld::GetBodyTransform(void* actorPtr, glm::vec3& outPosition, glm::vec3& outEulerAngles)
    {
        if (!actorPtr)
            return false;

        PxRigidActor* actor = reinterpret_cast<PxRigidActor*>(actorPtr);
        if (!actor)
            return false;

        PxTransform pose = actor->getGlobalPose();
        PxVec3 p = pose.p;
        PxQuat q = pose.q;

        outPosition = glm::vec3(p.x, p.y, p.z);

        // Convert PxQuat (x,y,z,w) -> glm::quat(w,x,y,z)
        glm::quat gq(q.w, q.x, q.y, q.z);
        outEulerAngles = glm::eulerAngles(gq);

        return true;
    }

    void PhysicsWorld::DestroyBody(void* actorPtr)
    {
        if (!actorPtr || !m_Impl || !m_Impl->Scene)
            return;

        PxRigidActor* actor = reinterpret_cast<PxRigidActor*>(actorPtr);
        m_Impl->Scene->removeActor(*actor);
        actor->release();
    }

} // namespace Titan::Physics3D
