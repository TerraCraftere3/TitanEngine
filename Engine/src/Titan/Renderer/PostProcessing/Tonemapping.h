#pragma once

#include "Titan/PCH.h"
#include "Titan/Renderer/PipelineState.h"
#include "Titan/Renderer/PostProcessing.h" // For PostFX base
#include "TonemappingTypes.h"              // TonemappingOperator enum

namespace Titan
{
    // TonemappingOperator enum + helper now defined in TonemappingTypes.h

    class TonemappingEffect : public PostFX
    {
    public:
        TonemappingEffect() : PostFX("Tonemapping") {};

        virtual void OnAttach() override;
        virtual void OnDetach() override;
        void Render(const PostFXInput& input) override;

    private:
        Ref<Shader> m_Shader;
        Ref<UniformBuffer> m_UniformBuffer;
        Ref<PipelineState> m_Pipeline;
    };
} // namespace Titan
