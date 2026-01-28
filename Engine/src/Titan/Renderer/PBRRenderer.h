#pragma once

#include "RHI/Cubemap.h"
#include "RHI/Framebuffer.h"
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

        static void Render(Ref<Framebuffer> input, PBRSceneData data, Ref<Cubemap> irradiance = nullptr);
    };

} // namespace Titan