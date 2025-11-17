#pragma once

#include "Titan/Renderer/RendererAPI.h"

namespace Titan
{

    class OpenGLRendererAPI : public RendererAPI
    {
    public:
        virtual void Init() override;

        virtual void SetDepthFunc(DepthFunc function) override;
        virtual void SetClearColor(const glm::vec4& color) override;
        virtual void Clear() override;

        virtual void DrawArrays(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) override;
        virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount) override;
        virtual void DrawIndexedInstanced(const Ref<VertexArray>& vertexArray, uint32_t indexCount,
                                          uint32_t instanceCount) override;
        virtual void DrawIndexedInstancedBaseIndex(const Ref<VertexArray>& vertexArray, uint32_t indexCount,
                                                   uint32_t instanceCount, uint32_t baseIndex) override;
        virtual void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) override;

        virtual void SetLineWidth(float width) override;
        virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
    };

} // namespace Titan