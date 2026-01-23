#pragma once

#include "Titan/Renderer/RendererAPI.h"

#include <string>

namespace Titan
{

    class OpenGLRendererAPI : public RendererAPI
    {
    public:
        virtual void Init() override;

        virtual void ResetStats() override;
        virtual Statistics GetStats() override;

        virtual void BeginFrame() override;
        virtual void EndFrame() override;

        virtual void BeginRenderPass(Ref<Framebuffer> framebuffer, std::string debugName) override;
        virtual void EndRenderPass() override;

        virtual void Clear(const glm::vec4& color) override;

        virtual void DrawArrays(uint32_t vertexCount) override;
        virtual void DrawIndexed(uint32_t indexCount) override;
        virtual void DrawIndexedInstanced(uint32_t indexCount, uint32_t instanceCount) override;
        virtual void DrawIndexedInstancedBaseIndex(uint32_t indexCount, uint32_t instanceCount,
                                                   uint32_t baseIndex) override;
        virtual void DrawLines(uint32_t vertexCount) override;

        virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

        virtual void* GetRawData() override;

        virtual Ref<Framebuffer> GetSwapchainTarget() override;

        virtual const RendererAPI::Backend& GetBackend() const override;

    private:
        Statistics m_Stats;
    };

} // namespace Titan