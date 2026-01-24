#pragma once

#include "Texture.h"
#include "Titan/PCH.h"

namespace Titan
{

    struct Material3D
    {
        std::string Name = "Material";
        glm::vec4 AlbedoColor = glm::vec4(1.0f);
        Ref<Texture2D> AlbedoTexture;
        Ref<Texture2D> EmissionTexture;
        Ref<Texture2D> MetallicTexture;
        Ref<Texture2D> RoughnessTexture;
        Ref<Texture2D> NormalTexture;
        Ref<Texture2D> AOTexture;
        glm::vec2 UVRepeat = glm::vec2(1.0f);

        std::string SourcePath = "";
        std::string InternalPath = "";

        Material3D() = default;
        Material3D(const Material3D&) = default;

        void TI_API Save();
        static Ref<Material3D> TI_API Create(const std::string& path);

        void SetInternalPath(const std::string& internalPath) { InternalPath = internalPath; }
        const std::string& GetInternalPath() const { return InternalPath; }
    };
} // namespace Titan