#pragma once

#include "Framebuffer.h"
#include "Titan/PCH.h"

namespace Titan
{
    struct PBRSceneData
    {
        glm::vec3 ViewPosition;
        bool HasDirectionalLight;
        glm::vec3 LightDirection;
        float Padding; // Padding to align to 16 bytes
    };

    class TI_API PBRRenderer
    {
    public:
        static void Init();
        static void Shutdown();

        static void Render(Ref<Framebuffer> input, PBRSceneData data);
    };

} // namespace Titan