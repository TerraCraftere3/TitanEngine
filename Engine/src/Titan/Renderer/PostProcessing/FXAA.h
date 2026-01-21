#pragma once

#include "Titan/PCH.h"
#include "Titan/Renderer/PipelineState.h"
#include "Titan/Renderer/PostProcessing.h"

namespace Titan
{
    class FXAAEffect : public PostFX
    {
    public:
        FXAAEffect() : PostFX("FXAA") {};

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        void Render(const PostFXInput& input) override;

    private:
        Ref<Shader> m_Shader;
        Ref<UniformBuffer> m_UniformBuffer;
        Ref<PipelineState> m_Pipeline;
    };
} // namespace Titan