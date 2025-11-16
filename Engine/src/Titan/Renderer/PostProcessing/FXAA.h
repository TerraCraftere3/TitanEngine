#pragma once

#include "Titan/PCH.h"
#include "Titan/Renderer/PostProcessing.h"

namespace Titan
{
    class FXAAEffect : public PostFX
    {
        public:
        FXAAEffect() : PostFX("FXAA Effect") {};

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        void Execute(RenderGraph& graph, const RenderPass& pass, Ref<Framebuffer> framebuffer) override;
    };
} // namespace Titan