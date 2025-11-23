#pragma once

#include <entt/entt.hpp>
#include "Titan/Core/Timestep.h"
#include "Titan/Core/UUID.h"
#include "Titan/PCH.h"
#include "Titan/Renderer/EditorCamera.h"

namespace Titan
{
    namespace Physics2D
    {
        class PhysicsWorld;
    }
} // namespace Titan

namespace Titan
{
    class Entity;

    class TI_API Scene
    {
    public:
        Scene();
        ~Scene();

        static Ref<Scene> Copy(Ref<Scene> other);

        Entity CreateEntity(const std::string& name = "");
        Entity CreateEntityWithUUID(UUID uuid, const std::string& name = "");
        void DestroyEntity(Entity entity);

        void OnRuntimeStart();
        void OnRuntimeStop();

        void OnSimulationStart();
        void OnSimulationStop();

        void OnUpdateRuntime(Timestep ts);
        void OnUpdateSimulation(Timestep ts, EditorCamera& camera);
        void OnUpdateEditor(Timestep ts, EditorCamera& camera);
        void OnViewportResize(uint32_t width, uint32_t height);

        void DuplicateEntity(Entity entity);

        Entity FindEntityByName(std::string_view name);
        Entity GetEntityByUUID(UUID uuid);
        Entity GetPrimaryCameraEntity();

        bool IsRunning() const { return m_IsRunning; }

        template <typename... Components>
        auto GetAllEntitiesWith()
        {
            return m_Registry.view<Components...>();
        }

    private:
        template <typename T>
        void OnComponentAdded(Entity entity, T& component);

        void OnPhysics2DStart();
        void OnPhysics2DStop();
        void UpdateConstraints();

        // Update world transforms for entities that have parent/child relations
        void UpdateTransforms();
        void UpdateTransformRecursive(entt::entity entity, const glm::mat4& parentTransform);
        // Set/clear parent-child relationships
        void SetParent(Entity child, Entity parent);
        void RemoveParent(Entity child);

    private:
        entt::registry m_Registry;
        uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
        bool m_IsRunning = false;

        Physics2D::PhysicsWorld* m_PhysicsWorld = nullptr;

        std::unordered_map<UUID, entt::entity> m_EntityMap;

        friend class Entity;
        friend class SceneSerializer;
        friend class SceneHierarchyPanel;
    };

} // namespace Titan