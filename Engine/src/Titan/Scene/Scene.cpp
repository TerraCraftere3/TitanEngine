#include "Scene.h"
#include "Components.h"
#include "Entity.h"
#include "Titan/PCH.h"
#include "Titan/Renderer/RenderCommand.h"
#include "Titan/Renderer/Renderer2D.h"
#include "Titan/Renderer/Renderer3D.h"
#include "Titan/Scripting/ScriptEngine.h"

#include "box2d/box2d.h"

namespace Titan
{

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

    Scene::Scene() {}

    Scene::~Scene()
    {
        delete m_PhysicsWorld;
        m_PhysicsWorld = nullptr;
    }

    template <typename Component>
    static void CopyComponent(entt::registry& dst, entt::registry& src,
                              const std::unordered_map<UUID, entt::entity>& enttMap)
    {
        auto view = src.view<Component>();
        for (auto e : view)
        {
            UUID uuid = src.get<IDComponent>(e).ID;
            TI_CORE_ASSERT(enttMap.find(uuid) != enttMap.end());
            entt::entity dstEnttID = enttMap.at(uuid);

            auto& component = src.get<Component>(e);
            dst.emplace_or_replace<Component>(dstEnttID, component);
        }
    }

    template <typename Component>
    static void CopyComponentIfExists(Entity dst, Entity src)
    {
        if (src.HasComponent<Component>())
            dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
    }

    Ref<Scene> Scene::Copy(Ref<Scene> other)
    {
        Ref<Scene> newScene = CreateRef<Scene>();

        newScene->m_ViewportWidth = other->m_ViewportWidth;
        newScene->m_ViewportHeight = other->m_ViewportHeight;

        auto& srcSceneRegistry = other->m_Registry;
        auto& dstSceneRegistry = newScene->m_Registry;
        std::unordered_map<UUID, entt::entity> enttMap;

        auto idView = srcSceneRegistry.view<IDComponent>();
        for (auto e : idView)
        {
            UUID uuid = srcSceneRegistry.get<IDComponent>(e).ID;
            const auto& name = srcSceneRegistry.get<TagComponent>(e).Tag;
            Entity newEntity = newScene->CreateEntityWithUUID(uuid, name);
            enttMap[uuid] = (entt::entity)newEntity;
        }

        CopyComponent<TransformComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<SpriteRendererComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<CircleRendererComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<MeshRendererComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<CameraComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<NativeScriptComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<Rigidbody2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<BoxCollider2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<CircleCollider2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
        CopyComponent<ScriptComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);

        return newScene;
    }

    Entity Scene::CreateEntity(const std::string& name)
    {
        return CreateEntityWithUUID(UUID(), name);
    }

    Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
    {
        Entity entity(m_Registry.create(), this);
        entity.AddComponent<IDComponent>(uuid);
        entity.AddComponent<TransformComponent>();
        auto& tag = entity.AddComponent<TagComponent>();
        tag.Tag = name.empty() ? "Entity" : name;

        m_EntityMap[uuid] = entity;

        return entity;
    }

    void Scene::DuplicateEntity(Entity entity)
    {
        std::string name = entity.GetName();
        Entity newEntity = CreateEntity(name);

        CopyComponentIfExists<TransformComponent>(newEntity, entity);
        CopyComponentIfExists<SpriteRendererComponent>(newEntity, entity);
        CopyComponentIfExists<CircleRendererComponent>(newEntity, entity);
        CopyComponentIfExists<MeshRendererComponent>(newEntity, entity);
        CopyComponentIfExists<CameraComponent>(newEntity, entity);
        CopyComponentIfExists<NativeScriptComponent>(newEntity, entity);
        CopyComponentIfExists<Rigidbody2DComponent>(newEntity, entity);
        CopyComponentIfExists<BoxCollider2DComponent>(newEntity, entity);
        CopyComponentIfExists<CircleCollider2DComponent>(newEntity, entity);
        CopyComponentIfExists<ScriptComponent>(newEntity, entity);
    }

    void Scene::DestroyEntity(Entity entity)
    {
        m_EntityMap.erase(entity.GetUUID());
        m_Registry.destroy(entity);
    }

    void Scene::OnRuntimeStart()
    {
        TI_PROFILE_FUNCTION();
        m_IsRunning = true;

        OnPhysics2DStart();

        {
            ScriptEngine::OnRuntimeStart(this);
            // Instantiate all script entities

            auto view = m_Registry.view<ScriptComponent>();
            for (auto e : view)
            {
                Entity entity = {e, this};
                ScriptEngine::OnCreateEntity(entity);
            }
        }
    }

    void Scene::OnRuntimeStop()
    {
        TI_PROFILE_FUNCTION();
        m_IsRunning = false;

        OnPhysics2DStop();

        ScriptEngine::OnRuntimeStop();
    }

    void Scene::OnSimulationStart()
    {
        TI_PROFILE_FUNCTION();
        OnPhysics2DStart();
    }

    void Scene::OnSimulationStop()
    {
        TI_PROFILE_FUNCTION();
        OnPhysics2DStop();
    }

    void Scene::OnUpdateRuntime(Timestep ts)
    {
        TI_PROFILE_FUNCTION()

        // C# SCRIPTS
        {
            auto view = GetAllEntitiesWith<ScriptComponent>();
            for (auto e : view)
            {
                Entity entity = {e, this};
                ScriptEngine::OnUpdateEntity(entity, ts);
            }
        }

        // NATIVE SCRIPTS
        {
            GetAllEntitiesWith<NativeScriptComponent>().each(
                [=](auto entity, auto& nsc)
                {
                    if (!nsc.Instance)
                    {
                        nsc.Instance = nsc.InstantiateScript();
                        nsc.Instance->m_Entity = Entity{entity, this};

                        nsc.Instance->OnCreate();
                    }

                    nsc.Instance->OnUpdate(ts);
                });
        }

        // PHYSICS
        {
            const int32_t velocityIterations = 6;
            const int32_t positionIterations = 2;
            m_PhysicsWorld->Step(ts, velocityIterations, positionIterations);

            auto view = GetAllEntitiesWith<Rigidbody2DComponent>();
            for (auto e : view)
            {
                Entity entity = {e, this};
                auto& transform = entity.GetComponent<TransformComponent>();
                auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

                b2Body* body = (b2Body*)rb2d.RuntimeBody;
                const auto& position = body->GetPosition();
                transform.Translation.x = position.x;
                transform.Translation.y = position.y;
                transform.Rotation.z = body->GetAngle();
            }
        }

        // FIND CAMERA
        Camera* mainCamera = nullptr;
        glm::mat4 cameraTransform;
        {
            auto view = GetAllEntitiesWith<TransformComponent, CameraComponent>();
            for (auto entity : view)
            {
                auto& transform = view.get<TransformComponent>(entity);
                auto& camera = view.get<CameraComponent>(entity);
                if (camera.Primary)
                {
                    mainCamera = &camera.Camera;
                    cameraTransform = transform.GetTransform();
                    break;
                }
            }
        }

        // RENDER
        if (mainCamera)
        {
            RenderCommand::SetClearColor({173.0f / 255.0f, 216.0f / 255.0f, 230.0f / 255.0f, 1.0f});
            RenderCommand::Clear();
            RenderScene(mainCamera->GetProjection() * glm::inverse(cameraTransform), glm::vec3(cameraTransform[3]));
        }
    }

    void Scene::OnUpdateSimulation(Timestep ts, EditorCamera& camera)
    {
        TI_PROFILE_FUNCTION();
        RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1.0f});
        RenderCommand::Clear();
        RenderCommand::SetLineWidth(2.0f);

        {
            const int32_t velocityIterations = 6;
            const int32_t positionIterations = 2;
            m_PhysicsWorld->Step(ts, velocityIterations, positionIterations);

            auto view = GetAllEntitiesWith<Rigidbody2DComponent>();
            for (auto e : view)
            {
                Entity entity = {e, this};
                auto& transform = entity.GetComponent<TransformComponent>();
                auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

                b2Body* body = (b2Body*)rb2d.RuntimeBody;
                const auto& position = body->GetPosition();
                transform.Translation.x = position.x;
                transform.Translation.y = position.y;
                transform.Rotation.z = body->GetAngle();
            }
        }

        RenderScene(camera.GetViewProjection(), camera.GetPosition(), true);
    }

    void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera)
    {
        TI_PROFILE_FUNCTION();
        RenderCommand::SetClearColor({0.1f, 0.1f, 0.1f, 1.0f});
        RenderCommand::Clear();
        RenderCommand::SetLineWidth(2.0f);

        RenderScene(camera.GetViewProjection(), camera.GetPosition(), true);
    }

    void Scene::OnViewportResize(uint32_t width, uint32_t height)
    {
        if (m_ViewportWidth == width && m_ViewportHeight == height)
            return;

        m_ViewportWidth = width;
        m_ViewportHeight = height;

        auto view = GetAllEntitiesWith<CameraComponent>();
        for (auto entity : view)
        {
            auto& cameraComponent = view.get<CameraComponent>(entity);
            if (!cameraComponent.FixedAspectRatio)
                cameraComponent.Camera.SetViewportSize(width, height);
        }
    }

    Entity Scene::FindEntityByName(std::string_view name)
    {
        auto view = m_Registry.view<TagComponent>();
        for (auto entity : view)
        {
            const TagComponent& tc = view.get<TagComponent>(entity);
            if (tc.Tag == name)
                return Entity{entity, this};
        }
        return {};
    }

    Entity Scene::GetEntityByUUID(UUID uuid)
    {
        if (m_EntityMap.find(uuid) != m_EntityMap.end())
            return {m_EntityMap.at(uuid), this};

        TI_CORE_WARN("Couldnt find entity {}", (size_t)uuid);
        return {};
    }

    Entity Scene::GetPrimaryCameraEntity()
    {
        auto view = GetAllEntitiesWith<CameraComponent>();
        for (auto entity : view)
        {
            const auto& camera = view.get<CameraComponent>(entity);
            if (camera.Primary)
                return Entity{entity, this};
        }
        return {};
    }

    void Scene::OnPhysics2DStart()
    {
        TI_PROFILE_FUNCTION();
        m_PhysicsWorld = new b2World({0.0f, -9.8f});

        auto view = m_Registry.view<Rigidbody2DComponent>();
        for (auto e : view)
        {
            Entity entity = {e, this};
            auto& transform = entity.GetComponent<TransformComponent>();
            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

            b2BodyDef bodyDef;
            bodyDef.type = Rigidbody2DTypeToBox2DBody(rb2d.Type);
            bodyDef.position.Set(transform.Translation.x, transform.Translation.y);
            bodyDef.angle = transform.Rotation.z;

            b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
            body->SetFixedRotation(rb2d.FixedRotation);
            rb2d.RuntimeBody = body;

            if (entity.HasComponent<BoxCollider2DComponent>())
            {
                auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
                auto mat = bc2d.Material;

                b2PolygonShape boxShape;
                boxShape.SetAsBox(bc2d.Size.x * transform.Scale.x, bc2d.Size.y * transform.Scale.y);

                b2FixtureDef fixtureDef;
                fixtureDef.shape = &boxShape;
                fixtureDef.density = mat->Density;
                fixtureDef.friction = mat->Friction;
                fixtureDef.restitution = mat->Restitution;
                fixtureDef.restitutionThreshold = mat->RestitutionThreshold;
                body->CreateFixture(&fixtureDef);
            }

            if (entity.HasComponent<CircleCollider2DComponent>())
            {
                auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();
                auto mat = cc2d.Material;

                b2CircleShape circleShape;
                circleShape.m_p.Set(cc2d.Offset.x, cc2d.Offset.y);
                circleShape.m_radius = transform.Scale.x * cc2d.Radius;

                b2FixtureDef fixtureDef;
                fixtureDef.shape = &circleShape;
                fixtureDef.density = mat->Density;
                fixtureDef.friction = mat->Friction;
                fixtureDef.restitution = mat->Restitution;
                fixtureDef.restitutionThreshold = mat->RestitutionThreshold;
                body->CreateFixture(&fixtureDef);
            }
        }
    }

    void Scene::OnPhysics2DStop()
    {
        TI_PROFILE_FUNCTION();
        delete m_PhysicsWorld;
        m_PhysicsWorld = nullptr;
    }

    void Scene::RenderScene(const glm::mat4& viewProjection, const glm::vec3& viewPosition, bool drawOverlay)
    {
        TI_PROFILE_FUNCTION();
        Renderer2D::BeginScene(viewProjection);
        Renderer3D::BeginScene(viewProjection, viewPosition);

        {
            auto spriteView = GetAllEntitiesWith<TransformComponent, SpriteRendererComponent>();
            for (auto entity : spriteView)
            {
                auto [transform, sprite] = spriteView.get<TransformComponent, SpriteRendererComponent>(entity);

                if (sprite.Tex)
                    Renderer2D::DrawTransformedQuad(transform.GetTransform(), sprite.Tex, 1.0f, sprite.Color,
                                                    (uint32_t)entity);
                else
                    Renderer2D::DrawTransformedQuad(transform.GetTransform(), sprite.Color, (uint32_t)entity);
            }
        }
        {
            auto circleView = GetAllEntitiesWith<TransformComponent, CircleRendererComponent>();
            for (auto entity : circleView)
            {
                auto [transform, circle] = circleView.get<TransformComponent, CircleRendererComponent>(entity);

                Renderer2D::DrawCircle(transform.GetTransform(), circle.Color, circle.Thickness, circle.Fade,
                                       (uint32_t)entity);
            }
        }
        {
            auto meshView = GetAllEntitiesWith<TransformComponent, MeshRendererComponent>();
            for (auto entity : meshView)
            {
                auto [transform, meshComp] = meshView.get<TransformComponent, MeshRendererComponent>(entity);
                if (meshComp.MeshRef)
                    Renderer3D::DrawMesh(meshComp.MeshRef, meshComp.Material, transform.GetTransform(),
                                         (uint32_t)entity);
            }
        }
        if (drawOverlay)
        {
            {
                auto colliderView = GetAllEntitiesWith<TransformComponent, BoxCollider2DComponent>();
                for (auto entity : colliderView)
                {
                    auto [transform, collider] = colliderView.get<TransformComponent, BoxCollider2DComponent>(entity);

                    Renderer2D::DrawRect(transform.GetTransform(), glm::vec4(0.0f, 0.9f, 0.0f, 1.0));
                }
            }
            {
                auto colliderView = GetAllEntitiesWith<TransformComponent, CircleCollider2DComponent>();
                for (auto entity : colliderView)
                {
                    auto [transform, collider] =
                        colliderView.get<TransformComponent, CircleCollider2DComponent>(entity);

                    // Renderer2D::DrawCircle(transform.GetTransform(), glm::vec4(0, 1, 0, 1), 0.05f);
                    Renderer2D::DrawRect(transform.GetTransform(), glm::vec4(0.0f, 0.9f, 0.0f, 1.0));
                }
            }
        }

        Renderer2D::EndScene();
        Renderer3D::EndScene();
    }

    template <typename T>
    void Scene::OnComponentAdded(Entity entity, T& component)
    {
    }

    template <>
    void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
    {
        component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
    }
} // namespace Titan