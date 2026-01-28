#pragma once

#include "RHI/Framebuffer.h"
#include "RHI/PipelineState.h"
#include "RHI/Shader.h"
#include "RHI/UniformBuffer.h"
#include "Titan/PCH.h"

namespace Titan
{
    enum class BlitMode
    {
        ColorOnly = 0,     // Copy only the color attachment
        ColorAndEntity = 1 // Copy color + entity ID attachments
    };

    class TI_API Blit
    {
    public:
        static void Init();
        static void Shutdown();

        static void Execute(const Ref<Framebuffer>& source, const Ref<Framebuffer>& destination,
                            BlitMode mode = BlitMode::ColorOnly);

    private:
        struct BlitData
        {
            Ref<Shader> Shader;
            Ref<PipelineState> Pipeline;
            Ref<UniformBuffer> UniformBuffer;
        };

        static BlitData s_Data;
    };

} // namespace Titan
