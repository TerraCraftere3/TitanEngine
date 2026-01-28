#pragma once

#include "RHI/PipelineState.h"
#include "RHI/VertexArray.h"
#include "RenderCommand.h"

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
