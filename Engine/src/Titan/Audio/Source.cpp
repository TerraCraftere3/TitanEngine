#include "Source.h"
#include "AudioContext.h"
#include "Titan/Platform/OpenAL/OpenALSource.h"

namespace Titan
{
    Ref<AudioSource> AudioSource::Create()
    {
        switch (AudioContext::GetAPI())
        {
            case AudioAPI::None:
                TI_CORE_ASSERT(false, "Audio API is not supported!");
                return nullptr;
            case AudioAPI::OpenAL:
                return CreateRef<OpenALSource>();
        }
        TI_CORE_ASSERT(false, "Unknown Audio API!");
        return nullptr;
    }
} // namespace Titan