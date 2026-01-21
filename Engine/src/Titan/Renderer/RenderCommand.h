#pragma once

#include "RendererAPI.h"

namespace Titan
{
    class TI_API RenderCommand
    {
    public:
        inline static void Init() { s_RendererAPI->Init(); }

        inline static void ResetStats() { s_RendererAPI->ResetStats(); }

        inline static RendererAPI::Statistics GetStats() { return s_RendererAPI->GetStats(); }

        inline static void BeginRenderPass(Ref<Framebuffer> framebuffer, std::string debugName)
        {
            s_RendererAPI->BeginRenderPass(framebuffer, debugName);
        }

        inline static void EndRenderPass() { s_RendererAPI->EndRenderPass(); }

        inline static void SetClearColor(const glm::vec4& color) { s_RendererAPI->SetClearColor(color); }

        inline static void Clear() { s_RendererAPI->Clear(); }

        inline static void DrawArrays(uint32_t vertexCount = 0) { s_RendererAPI->DrawArrays(vertexCount); }

        inline static void DrawIndexed(uint32_t indexCount = 0) { s_RendererAPI->DrawIndexed(indexCount); }

        inline static void DrawIndexedInstanced(uint32_t indexCount, uint32_t instanceCount)
        {
            s_RendererAPI->DrawIndexedInstanced(indexCount, instanceCount);
        }

        inline static void DrawIndexedInstancedBaseIndex(uint32_t indexCount, uint32_t instanceCount,
                                                         uint32_t baseIndex)
        {
            s_RendererAPI->DrawIndexedInstancedBaseIndex(indexCount, instanceCount, baseIndex);
        }

        inline static void DrawLines(uint32_t vertexCount) { s_RendererAPI->DrawLines(vertexCount); }

        inline static void SetLineWidth(float width) { s_RendererAPI->SetLineWidth(width); }

        inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
        {
            s_RendererAPI->SetViewport(x, y, width, height);
        }

        inline static const RendererAPI::Backend& GetBackend() { return s_RendererAPI->GetBackend(); }

    private:
        static Scope<RendererAPI> s_RendererAPI;
    };

} // namespace Titan