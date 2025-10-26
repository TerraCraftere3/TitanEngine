#pragma once

#include "Camera.h"
#include "EditorCamera.h"
#include "Texture.h"
#include "Titan/PCH.h"

namespace Titan
{

    class TI_API Renderer2D
    {
    public:
        static void Init();
        static void Shutdown();

        static void BeginScene(const EditorCamera& camera);
        static void BeginScene(const Camera& camera, const glm::mat4& transform);
        static void EndScene();
        static void Flush();

        static Ref<Texture2D> GetWhiteTexture();

        // Quads
        static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color,
                             int entityID = -1);

        static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture,
                             float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f),
                             int entityID = -1);
        static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color,
                             int entityID = -1);
        static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture,
                             float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f),
                             int entityID = -1);

        static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec3& rotation,
                                    const glm::vec4& color, int entityID = -1);
        static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec3& rotation,
                                    const glm::vec4& color, int entityID = -1);
        static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec3& rotation,
                                    const Ref<Texture2D>& texture, float tilingFactor = 1.0f,
                                    const glm::vec4& tintColor = glm::vec4(1.0f), int entityID = -1);
        static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec3& rotation,
                                    const Ref<Texture2D>& texture, float tilingFactor = 1.0f,
                                    const glm::vec4& tintColor = glm::vec4(1.0f), int entityID = -1);

        static void DrawTransformedQuad(const glm::mat4& transform, const glm::vec4& color, int entityID = -1);
        static void DrawTransformedQuad(const glm::mat4& transform, const Ref<Texture2D>& texture,
                                        float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f),
                                        int entityID = -1);

        // Statistics
        struct Statistics
        {
            uint32_t DrawCalls = 0;
            uint32_t QuadCount = 0;

            uint32_t GetTotalDrawCalls() { return DrawCalls; }
            uint32_t GetTotalQuadCount() { return QuadCount; }
            uint32_t GetTotalVertexCount() { return QuadCount * 4; }
            uint32_t GetTotalIndexCount() { return QuadCount * 6; }
            uint32_t GetTotalTriangleCount() { return QuadCount * 2; }
        };
        static Statistics GetStats();
        static void ResetStats();

    private:
        static void FlushAndReset();
    };

} // namespace Titan