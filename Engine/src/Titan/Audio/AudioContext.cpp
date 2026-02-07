#include "AudioContext.h"
#include "Titan/Platform/OpenAL/OpenALContext.h"

namespace Titan
{
    Ref<AudioContext> AudioContext::Create()
    {
        switch (AudioContext::GetAPI())
        {
            case AudioAPI::None:
                TI_CORE_ASSERT(false, "Audio API is not supported!");
                return nullptr;
            case AudioAPI::OpenAL:
                return CreateRef<OpenALContext>();
        }

        TI_CORE_ASSERT(false, "Unknown Audio API!");
        return nullptr;
    }

    AudioAPI AudioContext::GetAPI()
    {
        return AudioAPI::OpenAL;
    }
} // namespace Titan