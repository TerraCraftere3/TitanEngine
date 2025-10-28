#include "Entity.h"
#include "Components.h"

namespace Titan
{
    Entity::Entity() : m_EntityHandle(entt::null), m_Scene(nullptr) {}

    Entity::Entity(entt::entity handle, Scene* scene) : m_EntityHandle(handle), m_Scene(scene) {}

    UUID Entity::GetUUID()
    {
        return GetComponent<IDComponent>().ID;
    }
    std::string Entity::GetName()
    {
        return GetComponent<TagComponent>().Tag;
    }
} // namespace Titan