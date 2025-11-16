#pragma once

#include "Cubemap.h"
#include "Titan/PCH.h"

namespace Titan
{
    class TI_API SkyboxRenderer
    {
    public:
        static void Init();
        static void Shutdown();

        static void Render(Ref<Cubemap> cubemap, glm::mat4 view, glm::mat4 projection);
    };

} // namespace Titan