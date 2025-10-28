#pragma once

#include <entt/entt.hpp>
#include "Titan/Core/Timestep.h"
#include "Titan/Core/UUID.h"
#include "Titan/PCH.h"
#include "Titan/Renderer/EditorCamera.h"

class b2World;

namespace Titan
{
    class Entity;

    class TI_API Scene
    {
    public:
        Scene();
        ~Scene();

        void Clear();

        Entity CreateEntity(const std::string& name = "");
        Entity CreateEntityWithUUID(UUID uuid, const std::string& name = "");
        void DestroyEntity(Entity entity);

        void OnRuntimeStart();
        void OnRuntimeStop();

        void OnUpdateRuntime(Timestep ts);
        void OnUpdateEditor(Timestep ts, EditorCamera& camera);
        void OnViewportResize(uint32_t width, uint32_t height);

        Entity GetPrimaryCameraEntity();

    private:
        template <typename T>
        void OnComponentAdded(Entity entity, T& component);

    private:
        entt::registry m_Registry;
        uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

        b2World* m_PhysicsWorld = nullptr;

        friend class Entity;
        friend class SceneSerializer;
        friend class SceneHierarchyPanel;
    };

} // namespace Titan