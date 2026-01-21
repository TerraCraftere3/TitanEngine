#pragma once

#include "Framebuffer.h"
#include "PipelineState.h"
#include "Shader.h"
#include "Titan/PCH.h"
#include "UniformBuffer.h"

namespace Titan
{
    enum class BlitMode
    {
        ColorOnly = 0,         // Copy only the color attachment
        ColorAndEntity = 1     // Copy color + entity ID attachments
    };

    class TI_API Blit
    {
    public:
        static void Init();
        static void Shutdown();

        static void Execute(const Ref<Framebuffer>& source, const Ref<Framebuffer>& destination, BlitMode mode = BlitMode::ColorOnly);

    private:
        struct BlitData
        {
            Ref<Shader> Shader;
            Ref<PipelineState> Pipeline;
        };

        static BlitData s_Data;
    };

} // namespace Titan
