#pragma once

#include "Titan/Core/Timestep.h"
#include "Titan/PCH.h"

namespace Titan
{
    class Entity;

    class TI_API Scene
    {
    public:
        Scene();
        ~Scene();

        Entity CreateEntity(const std::string& name = std::string());
        void DestroyEntity(Entity entity);

        void OnUpdate(Timestep ts);
        void OnViewportResize(uint32_t width, uint32_t height);

    private:
        template <typename T>
        void OnComponentAdded(Entity entity, T& component);

    private:
        entt::registry m_Registry;
        uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

        friend class Entity;
        friend class SceneSerializer;
        friend class SceneHierarchyPanel;
    };

} // namespace Titan