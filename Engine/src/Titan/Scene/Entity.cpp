#include "Entity.h"
#include "Components.h"

namespace Titan
{
    UUID Entity::GetUUID()
    {
        return GetComponent<IDComponent>().ID;
    }

    std::string Entity::GetName()
    {
        return GetComponent<TagComponent>().Tag;
    }

    Entity Entity::GetParent()
    {
        if (!HasComponent<RelationshipComponent>())
            return Entity();

        auto parentHandle = GetComponent<RelationshipComponent>().Parent;
        if (parentHandle == entt::null)
            return Entity();

        return Entity(parentHandle, m_Scene);
    }

    std::vector<Entity> Entity::GetChildren()
    {
        std::vector<Entity> children;
        if (!HasComponent<RelationshipComponent>())
            return children;

        const auto& vec = GetComponent<RelationshipComponent>().Children;
        for (auto child : vec)
            children.emplace_back(Entity(child, m_Scene));

        return children;
    }
} // namespace Titan