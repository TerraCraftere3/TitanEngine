#include "RenderGraph.h"
#include "Titan/PCH.h"
#include <algorithm>
#include <queue>
#include <set>

namespace Titan
{
    // ============================================================================
    // RenderPass Implementation
    // ============================================================================

    bool RenderPass::HasInput(const std::string& name) const
    {
        return std::find(m_Descriptor.Inputs.begin(), m_Descriptor.Inputs.end(), name) 
               != m_Descriptor.Inputs.end();
    }

    bool RenderPass::HasOutput(const std::string& name) const
    {
        return std::find(m_Descriptor.Outputs.begin(), m_Descriptor.Outputs.end(), name) 
               != m_Descriptor.Outputs.end();
    }

    // ============================================================================
    // RenderGraph Implementation
    // ============================================================================

    void RenderGraph::RegisterResource(const ResourceDescriptor& desc)
    {
        TI_CORE_ASSERT(m_Resources.find(desc.Name) == m_Resources.end(), 
                      "Resource already registered: {0}", desc.Name);

        auto resource = CreateRef<RenderResource>(desc);
        m_Resources[desc.Name] = resource;

        if (!desc.Persistent)
        {
            m_TransientResources.push_back(desc.Name);
        }

        m_Compiled = false;
    }

    void RenderGraph::SetExternalResource(const std::string& name, void* handle)
    {
        auto it = m_Resources.find(name);
        if (it != m_Resources.end())
        {
            it->second->SetHandle(handle);
        }
        else
        {
            TI_CORE_WARN("Attempting to set external resource that doesn't exist: {0}", name);
        }
    }

    Ref<RenderResource> RenderGraph::GetResource(const std::string& name)
    {
        auto it = m_Resources.find(name);
        if (it != m_Resources.end())
            return it->second;
        
        TI_CORE_WARN("Resource not found: {0}", name);
        return nullptr;
    }

    Ref<Framebuffer> RenderGraph::GetFramebuffer(const std::string& name)
    {
        auto it = m_Framebuffers.find(name);
        if (it != m_Framebuffers.end())
            return it->second;
        
        return nullptr;
    }

std::unordered_map<std::string, Ref<Framebuffer>> RenderGraph::GetFramebuffers()
{
    return m_Framebuffers;
}


    RenderPass& RenderGraph::AddPass(const RenderPassDescriptor& desc, RenderPass::ExecuteFunc executeFunc)
    {
        TI_CORE_ASSERT(m_PassMap.find(desc.Name) == m_PassMap.end(), 
                      "Pass already exists: {0}", desc.Name);

        auto pass = CreateRef<RenderPass>(desc, executeFunc);
        m_Passes.push_back(pass);
        m_PassMap[desc.Name] = pass;

        m_Compiled = false;

        return *pass;
    }

    void RenderGraph::RemovePass(const std::string& name)
    {
        auto it = m_PassMap.find(name);
        if (it != m_PassMap.end())
        {
            auto pass = it->second;
            m_Passes.erase(std::remove(m_Passes.begin(), m_Passes.end(), pass), m_Passes.end());
            m_PassMap.erase(it);
            m_Compiled = false;
        }
    }

    void RenderGraph::Clear()
    {
        m_Passes.clear();
        m_PassMap.clear();
        m_ExecutionOrder.clear();
        DestroyTransientResources();
        m_Framebuffers.clear();
        m_Compiled = false;
    }

    void RenderGraph::Compile()
    {
        TI_PROFILE_FUNCTION();

        if (m_Compiled)
            return;

        // Validate graph structure
        ValidateGraph();

        // Check for cycles
        TI_CORE_ASSERT(!HasCycles(), "Render graph contains cycles!");

        // Sort passes topologically
        TopologicalSort();

        // Calculate resource lifetimes
        m_ResourceLifetimes.clear();
        for (uint32_t i = 0; i < m_ExecutionOrder.size(); ++i)
        {
            const auto& pass = m_ExecutionOrder[i];
            
            // Update lifetimes for inputs
            for (const auto& input : pass->GetInputs())
            {
                auto& lifetime = m_ResourceLifetimes[input];
                lifetime.FirstUse = min(lifetime.FirstUse, i);
                lifetime.LastUse = max(lifetime.LastUse, i);
            }
            
            // Update lifetimes for outputs
            for (const auto& output : pass->GetOutputs())
            {
                auto& lifetime = m_ResourceLifetimes[output];
                lifetime.FirstUse = min(lifetime.FirstUse, i);
                lifetime.LastUse = max(lifetime.LastUse, i);
            }
        }

        // Create physical resources
        CreatePhysicalResources();

        m_Compiled = true;

        TI_CORE_INFO("RenderGraph compiled successfully: {0} passes, {1} resources", 
                     m_ExecutionOrder.size(), m_Resources.size());
    }

    void RenderGraph::Execute()
    {
        TI_PROFILE_FUNCTION();
        
        if (!m_Compiled)
        {
            TI_CORE_WARN("Executing uncompiled RenderGraph - compiling now");
            Compile();
        }

        // Allocate transient resources at the start of frame
        AllocateTransientResources();

        // Execute passes in order
        for (auto& pass : m_ExecutionOrder)
        {
            TI_PROFILE_SCOPE(pass->GetName().c_str());
            pass->Execute(*this);
        }

        // Deallocate transient resources at end of frame
        DeallocateTransientResources();
    }

    void RenderGraph::Resize(uint32_t width, uint32_t height)
    {
        if (m_Width == width && m_Height == height)
            return;

        m_Width = width;
        m_Height = height;

        // Resize all framebuffers
        for (auto& [name, fb] : m_Framebuffers)
        {
            auto resource = GetResource(name);
            if (resource && !resource->GetDescriptor().Persistent)
            {
                fb->Resize(width, height);
            }
        }

        // Update resource descriptors
        for (auto& [name, resource] : m_Resources)
        {
            auto& desc = const_cast<ResourceDescriptor&>(resource->GetDescriptor());
            if (desc.Width == 0 || desc.Height == 0)
            {
                desc.Width = width;
                desc.Height = height;
            }
        }
    }

    RenderGraph::Statistics RenderGraph::GetStatistics() const
    {
        Statistics stats;
        stats.PassCount = static_cast<uint32_t>(m_Passes.size());
        stats.ResourceCount = static_cast<uint32_t>(m_Resources.size());
        stats.TransientResources = static_cast<uint32_t>(m_TransientResources.size());
        stats.PersistentResources = stats.ResourceCount - stats.TransientResources;
        return stats;
    }

    // ============================================================================
    // Private Helper Methods
    // ============================================================================

void RenderGraph::CreatePhysicalResources()
{
    TI_PROFILE_FUNCTION();

    for (auto& [name, resource] : m_Resources)
    {
        const auto& desc = resource->GetDescriptor();

        // Skip if external resource already set
        if (resource->GetHandle() != nullptr)
            continue;

        if (desc.Type == ResourceType::Texture2D)
        {
            FramebufferSpecification fbSpec;
            fbSpec.Width = desc.Width > 0 ? desc.Width : m_Width;
            fbSpec.Height = desc.Height > 0 ? desc.Height : m_Height;
            fbSpec.Samples = desc.Samples;

            // Use either the multiple formats or the single one
            if (!desc.AttachmentFormats.empty())
            {
                FramebufferAttachmentSpecification attachements;
                for(auto at : desc.AttachmentFormats){
                    attachements.Attachments.push_back({at});
                }
                fbSpec.Attachments = attachements;
            }
            else
            {
                if (desc.Format == FramebufferTextureFormat::Depth || 
                    desc.Format == FramebufferTextureFormat::DEPTH24STENCIL8)
                    fbSpec.Attachments = {FramebufferTextureFormat::RGBA8, desc.Format};
                else
                    fbSpec.Attachments = {desc.Format, FramebufferTextureFormat::Depth};
            }

            auto fb = Framebuffer::Create(fbSpec);
            m_Framebuffers[name] = fb;
            resource->SetHandle(fb.get());
        }
    }
}


    void RenderGraph::DestroyTransientResources()
    {
        for (const auto& name : m_TransientResources)
        {
            m_Framebuffers.erase(name);
            auto resource = GetResource(name);
            if (resource)
                resource->SetHandle(nullptr);
        }
    }

void RenderGraph::TopologicalSort()
{
    TI_PROFILE_FUNCTION();

    m_ExecutionOrder.clear();

    // Build dependency graph
    std::unordered_map<std::string, std::vector<std::string>> adjacencyList;
    std::unordered_map<std::string, int> inDegree;

    for (const auto& pass : m_Passes)
    {
        const auto& name = pass->GetName();
        inDegree[name] = 0;
        adjacencyList[name] = {};
    }

    // Calculate dependencies based on resource usage
    for (const auto& pass : m_Passes)
    {
        const auto& outputs = pass->GetOutputs();

        for (const auto& otherPass : m_Passes)
        {
            if (pass == otherPass)
                continue;

            const auto& inputs = otherPass->GetInputs();
            const auto& otherOutputs = otherPass->GetOutputs();

            // If otherPass reads what pass writes, pass must execute first
            // BUT skip adding the edge if otherPass also writes that same resource
            for (const auto& output : outputs)
            {
                if (std::find(inputs.begin(), inputs.end(), output) != inputs.end())
                {
                    // if otherPass also writes the same resource, skip (avoid mutual edges)
                    if (std::find(otherOutputs.begin(), otherOutputs.end(), output) != otherOutputs.end())
                        continue;

                    // avoid duplicate edges
                    auto& neighbours = adjacencyList[pass->GetName()];
                    if (std::find(neighbours.begin(), neighbours.end(), otherPass->GetName()) == neighbours.end())
                    {
                        neighbours.push_back(otherPass->GetName());
                        inDegree[otherPass->GetName()]++;
                    }
                }
            }
        }
    }

    // Kahn's algorithm
    std::queue<std::string> queue;
    for (const auto& [name, degree] : inDegree)
    {
        if (degree == 0)
            queue.push(name);
    }

    while (!queue.empty())
    {
        std::string current = queue.front();
        queue.pop();

        m_ExecutionOrder.push_back(m_PassMap[current]);

        for (const auto& neighbor : adjacencyList[current])
        {
            inDegree[neighbor]--;
            if (inDegree[neighbor] == 0)
                queue.push(neighbor);
        }
    }

    TI_CORE_ASSERT(m_ExecutionOrder.size() == m_Passes.size(),
                  "Topological sort failed - possible cycle detected");
}


    bool RenderGraph::HasCycles() const
{
    std::set<std::string> visited;
    std::set<std::string> recursionStack;

    std::function<bool(const std::string&)> dfs = [&](const std::string& passName) -> bool
    {
        visited.insert(passName);
        recursionStack.insert(passName);

        auto it = m_PassMap.find(passName);
        if (it == m_PassMap.end())
            return false;

        const auto& pass = it->second;
        const auto& outputs = pass->GetOutputs();

        // Find passes that depend on this pass
        for (const auto& otherPass : m_Passes)
        {
            if (pass == otherPass)
                continue;

            const auto& inputs = otherPass->GetInputs();
            const auto& otherOutputs = otherPass->GetOutputs();

            for (const auto& output : outputs)
            {
                if (std::find(inputs.begin(), inputs.end(), output) != inputs.end())
                {
                    // If otherPass also writes the same resource, treat as non-dependent here
                    if (std::find(otherOutputs.begin(), otherOutputs.end(), output) != otherOutputs.end())
                        continue;

                    const auto& nextName = otherPass->GetName();

                    if (recursionStack.find(nextName) != recursionStack.end())
                        return true; // Cycle detected

                    if (visited.find(nextName) == visited.end())
                    {
                        if (dfs(nextName))
                            return true;
                    }
                }
            }
        }

        recursionStack.erase(passName);
        return false;
    };

    for (const auto& pass : m_Passes)
    {
        const auto& name = pass->GetName();
        if (visited.find(name) == visited.end())
        {
            if (dfs(name))
                return true;
        }
    }

    return false;
}


    void RenderGraph::ValidateGraph() const
    {
        // Check that all referenced resources exist
        for (const auto& pass : m_Passes)
        {
            for (const auto& input : pass->GetInputs())
            {
                TI_CORE_ASSERT(m_Resources.find(input) != m_Resources.end(),
                              "Pass '{0}' references non-existent input resource '{1}'",
                              pass->GetName(), input);
            }

            for (const auto& output : pass->GetOutputs())
            {
                TI_CORE_ASSERT(m_Resources.find(output) != m_Resources.end(),
                              "Pass '{0}' references non-existent output resource '{1}'",
                              pass->GetName(), output);
            }
        }
    }

    void RenderGraph::AllocateTransientResources()
    {
        // In a more advanced implementation, this would handle memory aliasing
        // For now, resources are created during compilation
    }

    void RenderGraph::DeallocateTransientResources()
    {
        // In a more advanced implementation, this would release aliased memory
        // For now, resources persist for the lifetime of the graph
    }

    // ============================================================================
    // RenderGraphBuilder Implementation
    // ============================================================================

    RenderGraphBuilder& RenderGraphBuilder::CreateTexture(const std::string& name, 
                                                          FramebufferTextureFormat format,
                                                          uint32_t width, uint32_t height, 
                                                          uint32_t samples)
    {
        ResourceDescriptor desc;
        desc.Name = name;
        desc.Type = ResourceType::Texture2D;
        desc.Format = format;
        desc.Width = width;
        desc.Height = height;
        desc.Samples = samples;
        desc.Persistent = false;

        m_Graph.RegisterResource(desc);
        return *this;
    }

    RenderGraphBuilder& RenderGraphBuilder::CreatePersistentTexture(const std::string& name,
                                                                    FramebufferTextureFormat format,
                                                                    uint32_t width, uint32_t height,
                                                                    uint32_t samples)
    {
        ResourceDescriptor desc;
        desc.Name = name;
        desc.Type = ResourceType::Texture2D;
        desc.Format = format;
        desc.Width = width;
        desc.Height = height;
        desc.Samples = samples;
        desc.Persistent = true;

        m_Graph.RegisterResource(desc);
        return *this;
    }

RenderGraphBuilder& RenderGraphBuilder::CreateFramebuffer(
    const std::string& name,
    const std::vector<FramebufferTextureFormat>& attachments,
    uint32_t width, uint32_t height,
    uint32_t samples)
{
    ResourceDescriptor desc;
    desc.Name = name;
    desc.Type = ResourceType::Texture2D;
    desc.AttachmentFormats = attachments;
    desc.Width = width;
    desc.Height = height;
    desc.Samples = samples;
    desc.Persistent = false;

    m_Graph.RegisterResource(desc);
    return *this;
}


    RenderGraphBuilder& RenderGraphBuilder::AddRenderPass(const std::string& name,
                                                          const std::vector<std::string>& inputs,
                                                          const std::vector<std::string>& outputs,
                                                          RenderPass::ExecuteFunc executeFunc)
    {
        RenderPassDescriptor desc;
        desc.Name = name;
        desc.Inputs = inputs;
        desc.Outputs = outputs;

        m_Graph.AddPass(desc, executeFunc);
        return *this;
    }

    void RenderGraphBuilder::Build()
    {
        m_Graph.Compile();
    }

} // namespace Titan