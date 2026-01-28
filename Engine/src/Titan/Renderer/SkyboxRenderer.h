#pragma once

#include "RHI/Cubemap.h"
#include "Titan/PCH.h"

namespace Titan
{
    class TI_API SkyboxRenderer
    {
    public:
        static void Init();
        static void Shutdown();

        static void Render(Ref<Cubemap> cubemap, glm::mat4 view, glm::mat4 projection);
        static void Render(const glm::vec3& topColor, const glm::vec3& bottomColor, glm::mat4 view,
                           glm::mat4 projection);
    };

} // namespace Titan