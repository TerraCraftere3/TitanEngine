#include "Buffer.h"
#include "AudioContext.h"
#include "Titan/Platform/OpenAL/OpenALBuffer.h"

namespace Titan
{
    Ref<AudioBuffer> AudioBuffer::Create(const std::filesystem::path& filepath)
    {
        switch (AudioContext::GetAPI())
        {
            case AudioAPI::None:
                TI_CORE_ASSERT(false, "Audio API is not supported!");
                return nullptr;
            case AudioAPI::OpenAL:
                return CreateRef<OpenALBuffer>(filepath);
        }
        TI_CORE_ASSERT(false, "Unknown Audio API!");
        return nullptr;
    }
} // namespace Titan