#pragma once

#include "Titan/PCH.h"
#include "Titan/Renderer/Material.h"
#include "Titan/Renderer/RHI/PipelineState.h"
#include "Titan/Renderer/RHI/Texture.h"

namespace Titan
{
    class TI_API TerrainRenderer
    {
    public:
        static void Init();
        static void Shutdown();

        static void BeginScene(const glm::mat4& viewTransform, PolygonMode polygonMode = PolygonMode::Fill);
        static void EndScene();

        static void DrawTerrain(const Ref<Texture2D>& heightMap, const Ref<Material3D>& material,
                                const glm::mat4& transform, int entityID = -1);
    };

} // namespace Titan