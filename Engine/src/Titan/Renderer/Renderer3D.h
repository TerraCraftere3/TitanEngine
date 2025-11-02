#pragma once

#include "Camera.h"
#include "EditorCamera.h"
#include "Texture.h"
#include "Mesh.h"
#include "Titan/PCH.h"

namespace Titan
{

    class TI_API Renderer3D
    {
    public:
        static void Init();
        static void Shutdown();

        static void BeginScene(const EditorCamera& camera);
        static void BeginScene(const Camera& camera, const glm::mat4& transform);
        static void BeginScene(const glm::mat4& viewTransform);
        static void StartBatch();
        static void EndScene();
        static void Flush();

        static void DrawMesh(const Ref<Mesh>& mesh, const glm::mat4& transform, 
                             int entityID = -1);

        // Statistics
        struct Statistics
        {
            uint32_t DrawCalls = 0;
            uint32_t MeshCount = 0;
            uint32_t VertexCount = 0;

            uint32_t GetTotalDrawCalls() { return DrawCalls; }
            uint32_t GetTotalMeshCount() { return MeshCount; }
            uint32_t GetTotalVertexCount() { return VertexCount; }
        };
        static Statistics GetStats();
        static void ResetStats();

    private:
        static void FlushAndReset();
    };

} // namespace Titan