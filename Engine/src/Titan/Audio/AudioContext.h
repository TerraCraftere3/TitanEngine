#pragma once

#include "Titan/Core.h"
#include "Titan/PCH.h"

namespace Titan
{
    enum class AudioAPI
    {
        None = 0,
        OpenAL = 1
    };

    class AudioContext
    {
    public:
        virtual ~AudioContext() = default;

        static Ref<AudioContext> Create();

        static AudioAPI GetAPI();
    };

} // namespace Titan