#pragma once

#include "PipelineState.h"
#include "RenderCommand.h"
#include "VertexArray.h"

namespace Titan
{
    class TI_API FullscreenRenderer
    {
    public:
        static void Init();
        static void Shutdown();

        static void Render();

    private:
        struct Data
        {
            Ref<VertexArray> FullscreenQuadVAO;
            Ref<PipelineState> Pipeline;
        };

        static Data s_Data;
    };
} // namespace Titan
