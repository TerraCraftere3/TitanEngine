#pragma once

#include "Titan/PCH.h"
#include "RenderGraph.h"
#include "Framebuffer.h"
#include "Texture.h"
#include "Cubemap.h"
#include "VertexArray.h"
#include "Buffer.h"
#include "RenderCommand.h"
#include <vector>

namespace Titan
{
    class PostFX
    {
    public:
        inline PostFX(const char* Name) { m_Name = std::string(Name);}
        inline PostFX(std::string Name) { m_Name = Name; };
        virtual ~PostFX() = default;
        inline const std::string& GetName() const { return m_Name; }

        virtual void OnAttach() = 0;
        virtual void OnDetach() = 0;
        virtual void Execute(RenderGraph& graph, const RenderPass& pass, Ref<Framebuffer> framebuffer) = 0;

    protected:
        std::string m_Name;
    };

    class PostProcessingStack
    {
    public:
        PostProcessingStack() = default;

        void AddEffect(Ref<PostFX> effect)
        {
            effect->OnAttach();
                TI_CORE_TRACE("Attached Post Effect {}", effect->GetName());
            m_Effects.push_back(effect);
        }

        void Shutdown(){
            for (auto& effect : m_Effects)
            {
                effect->OnDetach();
                TI_CORE_TRACE("Detached Post Effect {}", effect->GetName());
            }
        }

        void RemoveEffect(const std::string& name)
        {
            m_Effects.erase(
                std::remove_if(m_Effects.begin(), m_Effects.end(),
                               [&](const Ref<PostFX>& effect) { return effect->GetName() == name; }),
                m_Effects.end());
        }

        void Execute(RenderGraph& graph, const RenderPass& pass, Ref<Framebuffer> framebuffer)
        {
            for (auto& effect : m_Effects)
            {
                effect->Execute(graph, pass, framebuffer);
            }
        }

        const std::vector<Ref<PostFX>>& GetEffects() const { return m_Effects; }

    private:
        std::vector<Ref<PostFX>> m_Effects;
    };
} // namespace Titan