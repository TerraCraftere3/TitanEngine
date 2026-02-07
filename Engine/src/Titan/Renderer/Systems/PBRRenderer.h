#pragma once

#include "Titan/PCH.h"
#include "Titan/Renderer/RHI/Cubemap.h"
#include "Titan/Renderer/RHI/Framebuffer.h"

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