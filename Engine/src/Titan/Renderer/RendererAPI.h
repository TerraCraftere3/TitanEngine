#pragma once

#include "Titan/PCH.h"
#include "Titan/Renderer/Framebuffer.h"

namespace Titan
{

    class TI_API RendererAPI
    {
    public:
        enum class API
        {
            None = 0,
            OpenGL = 1
        };

        struct Statistics
        {
            uint32_t DrawCalls = 0;
            uint32_t GetTotalDrawCalls() const { return DrawCalls; }
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

        virtual void ResetStats() = 0;
        virtual Statistics GetStats() = 0;

        virtual void BeginRenderPass(Ref<Framebuffer> framebuffer, std::string debugName) = 0;
        virtual void EndRenderPass() = 0;

        virtual void SetClearColor(const glm::vec4& color) = 0;
        virtual void Clear() = 0;

        virtual void DrawArrays(uint32_t vertexCount) = 0;
        virtual void DrawIndexed(uint32_t indexCount = 0) = 0;
        virtual void DrawIndexedInstanced(uint32_t indexCount, uint32_t instanceCount) = 0;
        virtual void DrawIndexedInstancedBaseIndex(uint32_t indexCount, uint32_t instanceCount, uint32_t baseIndex) = 0;
        virtual void DrawLines(uint32_t vertexCount) = 0;

        virtual void SetLineWidth(float width) = 0;

        virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
        virtual Ref<Framebuffer> GetSwapchainTarget() = 0;

        template <typename T>
        T GetData()
        {
            return *(static_cast<T*>(GetRawData()));
        }

        virtual const Backend& GetBackend() const = 0;
        inline static API GetAPI() { return s_API; }

    private:
        virtual void* GetRawData() = 0;
        static API s_API;
    };

} // namespace Titan