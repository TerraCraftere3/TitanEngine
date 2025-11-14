#pragma once

#include "Texture.h"
#include "Titan/PCH.h"

namespace Titan
{

    struct Material3D
    {
        glm::vec4 AlbedoColor = glm::vec4(1.0f);
        Ref<Texture2D> AlbedoTexture;
        Ref<Texture2D> MetallicTexture;
        Ref<Texture2D> RoughnessTexture;
        Ref<Texture2D> NormalTexture;
        Ref<Texture2D> AOTexture;
        glm::vec2 UVRepeat = glm::vec2(1.0f);
    };
} // namespace Titan