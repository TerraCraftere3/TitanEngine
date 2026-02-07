#pragma once

#include "Titan/PCH.h"
#include "Titan/Renderer/Camera.h"
#include "Titan/Renderer/EditorCamera.h"
#include "Titan/Renderer/Material.h"
#include "Titan/Renderer/Mesh.h"
#include "Titan/Renderer/RHI/PipelineState.h"
#include "Titan/Renderer/RHI/Texture.h"

namespace Titan
{
    class TI_API GeometryRenderer
    {
    public:
        static void Init();
        static void Shutdown();

        static void BeginScene(const glm::mat4& viewTransform, PolygonMode polygonMode = PolygonMode::Fill);
        static void ClearCache();
        static void ClearTextureCache();
        static void ClearMeshCache();
        static void EndScene();
        static void Flush();

        static void DrawMesh(const Ref<Mesh>& mesh, const glm::mat4& transform, int entityID = -1);

    private:
        static void FlushAndReset();
    };

} // namespace Titan