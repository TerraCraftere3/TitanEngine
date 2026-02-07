#include "Listener.h"
#include "AudioContext.h"
#include "Titan/Platform/OpenAL/OpenALListener.h"

namespace Titan
{
    Ref<AudioListener> AudioListener::Create()
    {
        switch (AudioContext::GetAPI())
        {
            case AudioAPI::None:
                TI_CORE_ASSERT(false, "Audio API is not supported!");
                return nullptr;
            case AudioAPI::OpenAL:
                return CreateRef<OpenALListener>();
        }
        TI_CORE_ASSERT(false, "Unknown Audio API!");
        return nullptr;
    }
} // namespace Titan