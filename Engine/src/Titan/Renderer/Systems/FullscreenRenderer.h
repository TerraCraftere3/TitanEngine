#pragma once

#include "Titan/Renderer/RHI/PipelineState.h"
#include "Titan/Renderer/RHI/VertexArray.h"
#include "Titan/Renderer/RenderCommand.h"

namespace Titan
{
    class TI_API FullscreenRenderer
    {
    public:
        static void Init();
        static void Shutdown();

        static void Render(const Ref<PipelineState>& pipeline);

    private:
        struct Data
        {
            Ref<VertexArray> FullscreenQuadVAO;
        };

        static Data s_Data;
    };
} // namespace Titan
