#pragma once

#include "VertexArray.h"
#include "RenderCommand.h"

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
        };

        static Data s_Data;
    };
}
