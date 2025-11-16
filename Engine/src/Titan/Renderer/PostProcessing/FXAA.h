#pragma once

#include "Titan/PCH.h"
#include "Titan/Renderer/PostProcessing.h"

namespace Titan
{
    class FXAAEffect : public PostFX
    {
        public:
            FXAAEffect() : PostFX("FXAA") {};

            virtual void OnAttach() override;
            virtual void OnDetach() override;
            void Execute(RenderGraph& graph, const RenderPass& pass, Ref<Framebuffer> framebuffer, Ref<Scene> scene,
                         PostFXComponent fxc) override;

        private:
            Ref<Shader> m_Shader;
            Ref<UniformBuffer> m_UniformBuffer;
    };
} // namespace Titan