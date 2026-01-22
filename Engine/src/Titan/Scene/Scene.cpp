#include "Scene.h"
#include "Components.h"
#include "Entity.h"
#include "EnttCompat.h"
#include "Titan/PCH.h"
#include "Titan/Renderer/GeometryRenderer.h"
#include "Titan/Renderer/RenderCommand.h"
#include "Titan/Renderer/Renderer2D.h"
#include "Titan/Scripting/ScriptEngine.h"

#include <algorithm>
#include "Titan/Physics/Physics2D/Physics2D.h"
#include "Titan/Physics/Physics3D/Physics3D.h"
#include "Titan/Physics/Physics3D/PhysicsWorld.h"

namespace Titan
{

    // Physics2D is abstracted into Titan::Physics2D::PhysicsWorld

    static glm::vec3 CalculateLookAtEuler3D(const glm::vec3& from, const glm::vec3& to)
    {
        glm::vec3 forward = glm::normalize(from - to);
        if (glm::length2(forward) < 1e-8f)
            return glm::vec3(0.0f);

        glm::vec3 up(0.0f, 1.0f, 0.0f); // world up

        // Build right and corrected up
        glm::vec3 right = glm::normalize(glm::cross(up, forward));
        glm::vec3 newUp = glm::cross(forward, right);

        // Construct rotation matrix with columns = local axes
        glm::mat3 rotMat(right, newUp, forward); // right, up, forward
        glm::quat q = glm::quat_cast(rotMat);

        return glm::eulerAngles(q); // returns XYZ Euler
    }

    Scene::Scene() {}

    Scene::~Scene()
    {
        delete m_PhysicsWorld;
        m_PhysicsWorld = nullptr;

        if (m_Physics3D)
        {
            // Destroy 3D bodies first
            auto view = m_Registry.view<RigidbodyComponent>();
            for (auto e : view)
            {
                Entity entity{e, this};
                auto& rb = entity.GetComponent<RigidbodyComponent>();
                if (rb.RuntimeBody)
                {
                    m_Physics3D->DestroyBody(rb.RuntimeBody);
                    rb.RuntimeBody = nullptr;
                }
            }

            delete m_Physics3D;
            m_Physics3D = nullptr;
        }
    }

    template <typename... Component>
    static void CopyComponent(entt::registry& dst, entt::registry& src,
                              const std::unordered_map<UUID, entt::entity>& enttMap)
    {
        (
            [&]()
            {
                auto view = src.view<Component>();
                for (auto it = view.rbegin(); it != view.rend(); ++it)
                {
                    entt::entity srcEntity = *it;
                    entt::entity dstEntity = enttMap.at(src.get<IDComponent>(srcEntity).ID);

                    auto& srcComponent = src.get<Component>(srcEntity);
                    dst.emplace_or_replace<Component>(dstEntity, srcComponent);
                }
            }(),
            ...);
    }

    template <typename... Component>
    static void CopyComponents(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src,
                               const std::unordered_map<UUID, entt::entity>& enttMap)
    {
        CopyComponent<Component...>(dst, src, enttMap);
    }

    template <typename... Component>
    static void CopyComponentIfExists(Entity dst, Entity src)
    {
        (
            [&]()
            {
                if (src.HasComponent<Component>())
                    dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
            }(),
            ...);
    }

    template <typename... Component>
    static void CopyComponentsIfExists(ComponentGroup<Component...>, Entity dst, Entity src)
    {
        CopyComponentIfExists<Component...>(dst, src);
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
        for (auto it = idView.rbegin(); it != idView.rend(); ++it)
        {
            entt::entity e = *it;
            UUID uuid = srcSceneRegistry.get<IDComponent>(e).ID;
            const auto& name = srcSceneRegistry.get<TagComponent>(e).Tag;
            Entity newEntity = newScene->CreateEntityWithUUID(uuid, name);
            enttMap[uuid] = (entt::entity)newEntity;
        }

        CopyComponents(AllComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap);

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

        CopyComponentsIfExists(AllComponents{}, newEntity, entity);
    }

    void Scene::DestroyEntity(Entity entity)
    {
        m_EntityMap.erase(entity.GetUUID());
        m_Registry.destroy(entity);
    }

    void Scene::OnRuntimeStart()
    {
        m_IsRunning = true;

        OnPhysics2DStart();
        OnPhysics3DStart();

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
        m_IsRunning = false;

        OnPhysics2DStop();
        OnPhysics3DStop();

        ScriptEngine::OnRuntimeStop();
    }

    void Scene::OnSimulationStart()
    {
        OnPhysics2DStart();
        OnPhysics3DStart();
    }

    void Scene::OnSimulationStop()
    {
        OnPhysics2DStop();
        OnPhysics3DStop();
    }

    void Scene::OnUpdateRuntime(Timestep ts)
    {
        UpdateTransforms();

        // C# SCRIPTS
        {
            auto view = GetAllEntitiesWith<ScriptComponent>();
            for (auto e : view)
            {
                Entity entity = {e, this};
                ScriptEngine::OnUpdateEntity(entity, ts);
            }
        }

        // PHYSICS 2D
        {
            const int32_t velocityIterations = 6;
            const int32_t positionIterations = 2;
            if (m_PhysicsWorld)
                m_PhysicsWorld->Step(ts, velocityIterations, positionIterations);

            auto view = GetAllEntitiesWith<Rigidbody2DComponent>();
            for (auto e : view)
            {
                Entity entity = {e, this};
                auto& transform = entity.GetComponent<TransformComponent>();
                auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

                void* body = rb2d.RuntimeBody;
                if (body)
                {
                    glm::vec2 position = m_PhysicsWorld->GetBodyPosition(body);
                    transform.Translation.x = position.x;
                    transform.Translation.y = position.y;
                    transform.Rotation.z = m_PhysicsWorld->GetBodyAngle(body);
                }
            }
        }

        // PHYSICS 3D
        {
            if (m_Physics3D)
                m_Physics3D->Step(ts);

            auto view3 = GetAllEntitiesWith<RigidbodyComponent>();
            for (auto e : view3)
            {
                Entity entity = {e, this};
                auto& transform = entity.GetComponent<TransformComponent>();
                auto& rb = entity.GetComponent<RigidbodyComponent>();

                void* body = rb.RuntimeBody;
                if (body)
                {
                    glm::vec3 pos(0.0f);
                    glm::vec3 euler(0.0f);
                    if (m_Physics3D->GetBodyTransform(body, pos, euler))
                    {
                        // Set world transform so renderer/scene use physics pose directly
                        glm::mat4 rot = glm::toMat4(glm::quat(euler));
                        glm::mat4 trans = glm::translate(glm::mat4(1.0f), pos);
                        glm::mat4 scale = glm::scale(glm::mat4(1.0f), transform.Scale);
                        transform.WorldTransform = trans * rot * scale;
                        transform.UseWorldTransform = true;
                    }
                }
            }
        }

        UpdateConstraints();
    }

    void Scene::OnUpdateSimulation(Timestep ts, EditorCamera& camera)
    {
        UpdateTransforms();

        // PHYSICS 2D
        {
            const int32_t velocityIterations = 6;
            const int32_t positionIterations = 2;
            if (m_PhysicsWorld)
                m_PhysicsWorld->Step(ts, velocityIterations, positionIterations);

            auto view = GetAllEntitiesWith<Rigidbody2DComponent>();
            for (auto e : view)
            {
                Entity entity = {e, this};
                auto& transform = entity.GetComponent<TransformComponent>();
                auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

                void* body = rb2d.RuntimeBody;
                if (body)
                {
                    glm::vec2 position = m_PhysicsWorld->GetBodyPosition(body);
                    transform.Translation.x = position.x;
                    transform.Translation.y = position.y;
                    transform.Rotation.z = m_PhysicsWorld->GetBodyAngle(body);
                }
            }
        }

        // PHYSICS 3D
        {
            if (m_Physics3D)
                m_Physics3D->Step(ts);

            auto view3 = GetAllEntitiesWith<RigidbodyComponent>();
            for (auto e : view3)
            {
                Entity entity = {e, this};
                auto& transform = entity.GetComponent<TransformComponent>();
                auto& rb = entity.GetComponent<RigidbodyComponent>();

                void* body = rb.RuntimeBody;
                if (body)
                {
                    glm::vec3 pos(0.0f);
                    glm::vec3 euler(0.0f);
                    if (m_Physics3D->GetBodyTransform(body, pos, euler))
                    {
                        glm::mat4 rot = glm::toMat4(glm::quat(euler));
                        glm::mat4 trans = glm::translate(glm::mat4(1.0f), pos);
                        glm::mat4 scale = glm::scale(glm::mat4(1.0f), transform.Scale);
                        transform.WorldTransform = trans * rot * scale;
                        transform.UseWorldTransform = true;
                    }
                }
            }
        }

        UpdateConstraints();
    }

    void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera)
    {
        UpdateTransforms();

        UpdateConstraints();
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
        m_PhysicsWorld = new Physics2D::PhysicsWorld({0.0f, -9.8f});

        auto view = m_Registry.view<Rigidbody2DComponent>();
        for (auto e : view)
        {
            Entity entity = {e, this};
            auto& transform = entity.GetComponent<TransformComponent>();
            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

            const BoxCollider2DComponent* box = nullptr;
            const CircleCollider2DComponent* circle = nullptr;
            if (entity.HasComponent<BoxCollider2DComponent>())
                box = &entity.GetComponent<BoxCollider2DComponent>();
            if (entity.HasComponent<CircleCollider2DComponent>())
                circle = &entity.GetComponent<CircleCollider2DComponent>();

            void* body = m_PhysicsWorld->CreateBody(rb2d, transform, box, circle);
            rb2d.RuntimeBody = body;
        }
    }

    void Scene::OnPhysics2DStop()
    {
        delete m_PhysicsWorld;
        m_PhysicsWorld = nullptr;
    }

    void Scene::OnPhysics3DStart()
    {
        m_Physics3D = new Physics3D::PhysicsWorld({0.0f, -9.8f, 0.0f});

        auto view = m_Registry.view<RigidbodyComponent>();
        for (auto e : view)
        {
            Entity entity = {e, this};
            auto& transform = entity.GetComponent<TransformComponent>();
            auto& rb = entity.GetComponent<RigidbodyComponent>();

            const CubeColliderComponent* cube = nullptr;
            if (entity.HasComponent<CubeColliderComponent>())
                cube = &entity.GetComponent<CubeColliderComponent>();

            const SphereColliderComponent* sphere = nullptr;
            if (entity.HasComponent<SphereColliderComponent>())
                sphere = &entity.GetComponent<SphereColliderComponent>();

            void* body = m_Physics3D->CreateBody(rb, transform, cube, sphere);
            rb.RuntimeBody = body;
        }
    }

    void Scene::OnPhysics3DStop()
    {
        if (m_Physics3D)
        {
            auto view = m_Registry.view<RigidbodyComponent>();
            for (auto e : view)
            {
                Entity entity = {e, this};
                auto& rb = entity.GetComponent<RigidbodyComponent>();
                if (rb.RuntimeBody)
                {
                    m_Physics3D->DestroyBody(rb.RuntimeBody);
                    rb.RuntimeBody = nullptr;
                }
            }

            delete m_Physics3D;
            m_Physics3D = nullptr;
        }
    }

    void Scene::UpdateConstraints()
    {
        auto view = m_Registry.view<TransformComponent, LookAtComponent>();
        for (auto e : view)
        {
            auto& transform = view.get<TransformComponent>(e);
            auto& lookAt = view.get<LookAtComponent>(e);

            transform.Rotation = CalculateLookAtEuler3D(transform.Translation, lookAt.Position);
        }
    }

    void Scene::UpdateTransforms()
    {
        // Reset UseWorldTransform for all transforms by default
        auto allTransforms = m_Registry.view<TransformComponent>();
        for (auto e : allTransforms)
        {
            auto& tc = Titan::EnttCompat::registry_get<decltype(m_Registry), TransformComponent>(m_Registry, e);
            tc.UseWorldTransform = false;
            tc.WorldTransform = tc.GetLocalTransform();
        }

        // For entities with RelationshipComponent, find roots and recurse
        auto relView = m_Registry.view<RelationshipComponent, TransformComponent>();
        for (auto e : relView)
        {
            auto& rel = Titan::EnttCompat::registry_get<decltype(m_Registry), RelationshipComponent>(m_Registry, e);
            if (rel.Parent == entt::null)
            {
                UpdateTransformRecursive(e, glm::mat4(1.0f));
            }
        }
    }

    void Scene::UpdateTransformRecursive(entt::entity entity, const glm::mat4& parentTransform)
    {
        if (!m_Registry.valid(entity))
            return;

        auto& tc = Titan::EnttCompat::registry_get<decltype(m_Registry), TransformComponent>(m_Registry, entity);

        glm::mat4 local = tc.GetLocalTransform();
        glm::mat4 world = parentTransform * local;

        tc.WorldTransform = world;
        tc.UseWorldTransform = true;

        if (Titan::EnttCompat::registry_has<decltype(m_Registry), RelationshipComponent>(m_Registry, entity))
        {
            auto& rel =
                Titan::EnttCompat::registry_get<decltype(m_Registry), RelationshipComponent>(m_Registry, entity);
            for (auto child : rel.Children)
            {
                if (child != entt::null)
                    UpdateTransformRecursive(child, world);
            }
        }
    }

    void Scene::SetParent(Entity child, Entity parent)
    {
        if (!child || !parent)
            return;

        entt::entity childHandle = (entt::entity)child;
        entt::entity parentHandle = (entt::entity)parent;

        // Prevent cycles: ensure parent is not a descendant of child
        entt::entity cur = parentHandle;
        while (cur != entt::null)
        {
            if (cur == childHandle)
                return; // would create a cycle

            if (!Titan::EnttCompat::registry_has<decltype(m_Registry), RelationshipComponent>(m_Registry, cur))
                break;

            cur = Titan::EnttCompat::registry_get<decltype(m_Registry), RelationshipComponent>(m_Registry, cur).Parent;
        }

        // Remove from previous parent if any
        if (Titan::EnttCompat::registry_has<decltype(m_Registry), RelationshipComponent>(m_Registry, childHandle))
        {
            auto& childRel =
                Titan::EnttCompat::registry_get<decltype(m_Registry), RelationshipComponent>(m_Registry, childHandle);
            if (childRel.Parent != entt::null && childRel.Parent != parentHandle)
            {
                auto& prevRel = Titan::EnttCompat::registry_get<decltype(m_Registry), RelationshipComponent>(
                    m_Registry, childRel.Parent);
                auto& prevChildren = prevRel.Children;
                prevChildren.erase(std::remove(prevChildren.begin(), prevChildren.end(), childHandle),
                                   prevChildren.end());
            }
        }

        // Ensure relationship components exist
        if (!Titan::EnttCompat::registry_has<decltype(m_Registry), RelationshipComponent>(m_Registry, childHandle))
            m_Registry.emplace<RelationshipComponent>(childHandle);
        if (!Titan::EnttCompat::registry_has<decltype(m_Registry), RelationshipComponent>(m_Registry, parentHandle))
            m_Registry.emplace<RelationshipComponent>(parentHandle);

        auto& childRel2 =
            Titan::EnttCompat::registry_get<decltype(m_Registry), RelationshipComponent>(m_Registry, childHandle);
        auto& parentRel2 =
            Titan::EnttCompat::registry_get<decltype(m_Registry), RelationshipComponent>(m_Registry, parentHandle);

        childRel2.Parent = parentHandle;

        // add to parent's children if not present
        if (std::find(parentRel2.Children.begin(), parentRel2.Children.end(), childHandle) == parentRel2.Children.end())
            parentRel2.Children.push_back(childHandle);
    }

    void Scene::RemoveParent(Entity child)
    {
        if (!child)
            return;

        entt::entity childHandle = (entt::entity)child;
        if (!Titan::EnttCompat::registry_has<decltype(m_Registry), RelationshipComponent>(m_Registry, childHandle))
            return;

        auto& childRel =
            Titan::EnttCompat::registry_get<decltype(m_Registry), RelationshipComponent>(m_Registry, childHandle);
        if (childRel.Parent == entt::null)
            return;

        entt::entity parentHandle = childRel.Parent;
        if (Titan::EnttCompat::registry_has<decltype(m_Registry), RelationshipComponent>(m_Registry, parentHandle))
        {
            auto& parentRel =
                Titan::EnttCompat::registry_get<decltype(m_Registry), RelationshipComponent>(m_Registry, parentHandle);
            parentRel.Children.erase(std::remove(parentRel.Children.begin(), parentRel.Children.end(), childHandle),
                                     parentRel.Children.end());
        }

        childRel.Parent = entt::null;
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

    template void Scene::OnComponentAdded<IDComponent>(Entity, IDComponent&);
    template void Scene::OnComponentAdded<TagComponent>(Entity, TagComponent&);
    template void Scene::OnComponentAdded<TransformComponent>(Entity, TransformComponent&);
    template void Scene::OnComponentAdded<SpriteRendererComponent>(Entity, SpriteRendererComponent&);
    template void Scene::OnComponentAdded<CircleRendererComponent>(Entity, CircleRendererComponent&);
    template void Scene::OnComponentAdded<MeshRendererComponent>(Entity, MeshRendererComponent&);
    template void Scene::OnComponentAdded<DirectionalLightComponent>(Entity, DirectionalLightComponent&);
    template void Scene::OnComponentAdded<SkyboxComponent>(Entity, SkyboxComponent&);
    template void Scene::OnComponentAdded<CameraComponent>(Entity, CameraComponent&);
    template void Scene::OnComponentAdded<Rigidbody2DComponent>(Entity, Rigidbody2DComponent&);
    template void Scene::OnComponentAdded<BoxCollider2DComponent>(Entity, BoxCollider2DComponent&);
    template void Scene::OnComponentAdded<CircleCollider2DComponent>(Entity, CircleCollider2DComponent&);
    template void Scene::OnComponentAdded<ScriptComponent>(Entity, ScriptComponent&);
    template void Scene::OnComponentAdded<LookAtComponent>(Entity, LookAtComponent&);
} // namespace Titan