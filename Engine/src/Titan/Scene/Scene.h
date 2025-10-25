#pragma once

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

    private:
        entt::registry m_Registry;

		friend class Entity;
    };

} // namespace Titan