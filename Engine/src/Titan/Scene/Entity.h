#pragma once

#include <vector>
#include "Components.h"
#include "EnttCompat.h"
#include "Scene.h"
#include "Titan/PCH.h"

namespace Titan
{

    class TI_API Entity
    {
    public:
        Entity() = default;
        Entity(entt::entity handle, Scene* scene) : m_EntityHandle(handle), m_Scene(scene) {}

        Entity(const Entity& other) = default;

        // ============================
        // Component Management
        // ============================
        template <typename T, typename... Args>
        T& AddComponent(Args&&... args)
        {
            TI_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");
            T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
            m_Scene->OnComponentAdded<T>(*this, component);
            return component;
        }

        template <typename T>
        T& GetComponent()
        {
            TI_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
            return m_Scene->m_Registry.get<T>(m_EntityHandle);
        }

        template <typename T, typename... Args>
        T& AddOrReplaceComponent(Args&&... args)
        {
            T& component = m_Scene->m_Registry.emplace_or_replace<T>(m_EntityHandle, std::forward<Args>(args)...);
            m_Scene->OnComponentAdded<T>(*this, component);
            return component;
        }

        template <typename T>
        bool HasComponent()
        {
            return m_Scene->m_Registry.any_of<T>(m_EntityHandle);
        }

        template <typename T>
        void RemoveComponent()
        {
            TI_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
            m_Scene->m_Registry.remove<T>(m_EntityHandle);
        }

        // ============================
        // Operators
        // ============================
        explicit operator bool() const { return m_EntityHandle != entt::null; }
        operator entt::entity() const { return m_EntityHandle; }
        operator uint32_t() const { return static_cast<uint32_t>(m_EntityHandle); }

        // ============================
        // Entity Info
        // ============================
        UUID GetUUID();
        std::string GetName();

        // ============================
        // Hierarchy Helpers
        // ============================
        void SetParent(Entity parent)
        {
            TI_CORE_ASSERT(m_Scene, "Entity does not belong to a scene!");
            m_Scene->SetParent(*this, parent);
        }

        void RemoveParent()
        {
            TI_CORE_ASSERT(m_Scene, "Entity does not belong to a scene!");
            m_Scene->RemoveParent(*this);
        }

        Entity GetParent();

        std::vector<Entity> GetChildren();

        // ============================
        // Comparisons
        // ============================
        bool operator==(const Entity& other) const
        {
            return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
        }

        bool operator!=(const Entity& other) const { return !(*this == other); }

    private:
        entt::entity m_EntityHandle{entt::null};
        Scene* m_Scene = nullptr;
    };

} // namespace Titan
