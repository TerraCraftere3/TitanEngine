#pragma once

#include "Titan/PCH.h"
#include "Titan/Renderer/Depth.h"
#include "Titan/Renderer/VertexArray.h"

namespace Titan
{
    enum class PolygonMode
    {
        Fill = 0,
        Line = 1,
        Point = 2
    };

    class TI_API RendererAPI
    {
    public:
        enum class API
        {
            None = 0,
            OpenGL = 1
        };

        struct Backend
        {
            std::string Vendor;
            std::string Renderer;
            std::string Version;

            bool SupportsComputeShaders = false;
            bool SupportsTessellationShaders = false;
            bool SupportsGeometryShaders = false;

            uint32_t MaxTextureUnits = 0;
            uint32_t MaxTextureSize = 0;

            bool SupportsRaytracing = false;
        };

    public:
        virtual void Init() = 0;

        virtual void SetPolygonMode(PolygonMode mode) = 0;
        virtual void SetDepthFunc(DepthFunc function) = 0;
        virtual void SetClearColor(const glm::vec4& color) = 0;
        virtual void Clear() = 0;

        virtual void DrawArrays(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) = 0;
        virtual void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0) = 0;
        virtual void DrawIndexedInstanced(const Ref<VertexArray>& vertexArray, uint32_t indexCount,
                                          uint32_t instanceCount) = 0;
        virtual void DrawIndexedInstancedBaseIndex(const Ref<VertexArray>& vertexArray, uint32_t indexCount,
                                                   uint32_t instanceCount, uint32_t baseIndex) = 0;
        virtual void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) = 0;

        virtual void SetLineWidth(float width) = 0;

        virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;

        virtual const Backend& GetBackend() const = 0;
        inline static API GetAPI() { return s_API; }

    private:
        static API s_API;
    };

} // namespace Titan