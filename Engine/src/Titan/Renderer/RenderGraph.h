#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "Framebuffer.h"
#include "Titan/Core.h"

namespace Titan
{
    // Forward declarations
    class RenderGraph;
    class RenderPass;

    enum class ResourceType
    {
        Texture2D,
        TextureCube,
        Buffer
    };

    enum class ResourceUsage
    {
        None = 0,
        Read = 1 << 0,
        Write = 1 << 1,
        ReadWrite = Read | Write
    };

    struct ResourceDescriptor
    {
        std::string Name;
        ResourceType Type = ResourceType::Texture2D;
        std::vector<FramebufferTextureFormat> AttachmentFormats;
        FramebufferTextureFormat Format = FramebufferTextureFormat::RGBA8;
        uint32_t Width = 0;
        uint32_t Height = 0;
        uint32_t Samples = 1;
        bool Persistent = false; // If true, resource persists between frames
    };

    class RenderResource
    {
    public:
        RenderResource(const ResourceDescriptor& desc) : m_Descriptor(desc) {}
        virtual ~RenderResource() = default;

        const ResourceDescriptor& GetDescriptor() const { return m_Descriptor; }
        const std::string& GetName() const { return m_Descriptor.Name; }

        void* GetHandle() const { return m_Handle; }
        void SetHandle(void* handle) { m_Handle = handle; }

        uint32_t GetVersion() const { return m_Version; }
        void IncrementVersion() { m_Version++; }

    private:
        ResourceDescriptor m_Descriptor;
        void* m_Handle = nullptr;
        uint32_t m_Version = 0;
    };

    struct RenderPassDescriptor
    {
        std::string Name;
        std::vector<std::string> Inputs;  // Resource names to read
        std::vector<std::string> Outputs; // Resource names to write
        bool EnableDepthTest = true;
        bool EnableBlending = false;
    };

    class RenderPass
    {
    public:
        using ExecuteFunc = std::function<void(RenderGraph&, const RenderPass&)>;

        RenderPass(const RenderPassDescriptor& desc, ExecuteFunc executeFunc)
            : m_Descriptor(desc), m_ExecuteFunc(executeFunc)
        {
        }

        const RenderPassDescriptor& GetDescriptor() const { return m_Descriptor; }
        const std::string& GetName() const { return m_Descriptor.Name; }

        const std::vector<std::string>& GetInputs() const { return m_Descriptor.Inputs; }
        const std::vector<std::string>& GetOutputs() const { return m_Descriptor.Outputs; }

        void Execute(RenderGraph& graph) const { m_ExecuteFunc(graph, *this); }

        // Helper methods for querying resources during execution
        bool HasInput(const std::string& name) const;
        bool HasOutput(const std::string& name) const;

    private:
        RenderPassDescriptor m_Descriptor;
        ExecuteFunc m_ExecuteFunc;
    };

    // ============================================================================
    // Render Graph
    // ============================================================================

    class TI_API RenderGraph
    {
    public:
        RenderGraph() = default;
        ~RenderGraph() = default;

        // Resource Management
        void RegisterResource(const ResourceDescriptor& desc);
        void SetExternalResource(const std::string& name, void* handle);
        Ref<RenderResource> GetResource(const std::string& name);
        Ref<Framebuffer> GetFramebuffer(const std::string& name);
        std::unordered_map<std::string, Ref<Framebuffer>> GetFramebuffers();

        // Pass Management
        RenderPass& AddPass(const RenderPassDescriptor& desc, RenderPass::ExecuteFunc executeFunc);
        void RemovePass(const std::string& name);
        void Clear();

        // Execution
        void Compile();
        void Execute();
        void Resize(uint32_t width, uint32_t height);

        // Queries
        bool IsCompiled() const { return m_Compiled; }
        const std::vector<Ref<RenderPass>>& GetPasses() const { return m_ExecutionOrder; }

        // Statistics
        struct Statistics
        {
            uint32_t PassCount = 0;
            uint32_t ResourceCount = 0;
            uint32_t TransientResources = 0;
            uint32_t PersistentResources = 0;
        };
        Statistics GetStatistics() const;

    private:
        // Internal resource management
        void CreatePhysicalResources();
        void DestroyTransientResources();
        void TopologicalSort();
        bool HasCycles() const;
        void ValidateGraph() const;

        // Transient resource aliasing
        void AllocateTransientResources();
        void DeallocateTransientResources();

    private:
        // Resources
        std::unordered_map<std::string, Ref<RenderResource>> m_Resources;
        std::unordered_map<std::string, Ref<Framebuffer>> m_Framebuffers;
        std::vector<std::string> m_TransientResources;

        // Passes
        std::vector<Ref<RenderPass>> m_Passes;
        std::vector<Ref<RenderPass>> m_ExecutionOrder;
        std::unordered_map<std::string, Ref<RenderPass>> m_PassMap;

        // State
        bool m_Compiled = false;
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;

        // Resource lifetime tracking (for aliasing optimization)
        struct ResourceLifetime
        {
            uint32_t FirstUse = UINT32_MAX;
            uint32_t LastUse = 0;
        };
        std::unordered_map<std::string, ResourceLifetime> m_ResourceLifetimes;
    };

    class TI_API RenderGraphBuilder
    {
    public:
        RenderGraphBuilder(RenderGraph& graph) : m_Graph(graph) {}

        // Fluent API for building render passes
        RenderGraphBuilder& CreateTexture(const std::string& name, FramebufferTextureFormat format, uint32_t width,
                                          uint32_t height, uint32_t samples = 1);

        RenderGraphBuilder& CreatePersistentTexture(const std::string& name, FramebufferTextureFormat format,
                                                    uint32_t width, uint32_t height, uint32_t samples = 1);

        RenderGraphBuilder& CreateFramebuffer(const std::string& name,
                                              const std::vector<FramebufferTextureFormat>& attachments, uint32_t width,
                                              uint32_t height, uint32_t samples);

        RenderGraphBuilder& AddRenderPass(const std::string& name, const std::vector<std::string>& inputs,
                                          const std::vector<std::string>& outputs, RenderPass::ExecuteFunc executeFunc);

        void Build();

    private:
        RenderGraph& m_Graph;
    };

} // namespace Titan