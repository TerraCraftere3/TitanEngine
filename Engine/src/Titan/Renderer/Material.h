#pragma once

#include "Texture.h"
#include "Titan/PCH.h"

namespace Titan
{

    struct Material3D
    {
        glm::vec4 AlbedoColor = glm::vec4(1.0f);
        Ref<Texture2D> AlbedoTexture;
        float Metallic = 0.0f;
        Ref<Texture2D> MetallicTexture;
        float Roughness = 1.0f;
        Ref<Texture2D> RoughnessTexture;
        Ref<Texture2D> NormalTexture;
    };
} // namespace Titan