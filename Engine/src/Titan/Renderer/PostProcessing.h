#pragma once

#include <vector>
#include "Buffer.h"
#include "Cubemap.h"
#include "Framebuffer.h"
#include "RenderCommand.h"
#include "RenderGraph.h"
#include "Shader.h"
#include "ShaderStorageBuffer.h"
#include "Texture.h"
#include "Titan/PCH.h"
#include "Titan/Scene/Assets.h"
#include "Titan/Scene/Components.h"
#include "Titan/Scene/Scene.h"
#include "UniformBuffer.h"
#include "VertexArray.h"

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
        virtual void Execute(RenderGraph& graph, const RenderPass& pass, Ref<Framebuffer> framebuffer, Ref<Scene> scene,
                             PostFXComponent fxc) = 0;

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
            TI_CORE_INFO("Attached Post Effect \"{}\"", effect->GetName());
            m_Effects.push_back(effect);
        }

        void Shutdown(){
            for (auto& effect : m_Effects)
            {
                effect->OnDetach();
                TI_CORE_INFO("Detached Post Effect \"{}\"", effect->GetName());
            }
        }

        void RemoveEffect(const std::string& name)
        {
            m_Effects.erase(
                std::remove_if(m_Effects.begin(), m_Effects.end(),
                               [&](const Ref<PostFX>& effect) { return effect->GetName() == name; }),
                m_Effects.end());
        }

        void Execute(RenderGraph& graph, const RenderPass& pass, Ref<Framebuffer> framebuffer, Ref<Scene> scene)
        {
            PostFXComponent fxc;
            bool foundComponent = false;
            auto postFXView = scene->GetAllEntitiesWith<TransformComponent, PostFXComponent>();
            for (auto entity : postFXView)
            {
                auto [transform, postfxcomponent] = postFXView.get<TransformComponent, PostFXComponent>(entity);
                fxc = postfxcomponent;
                foundComponent = true;
                break;
            }

            if (!foundComponent)
                return;

            for (auto& effect : m_Effects)
            {
                effect->Execute(graph, pass, framebuffer, scene, fxc);
            }
        }

        const std::vector<Ref<PostFX>>& GetEffects() const { return m_Effects; }

    private:
        std::vector<Ref<PostFX>> m_Effects;
    };
} // namespace Titan