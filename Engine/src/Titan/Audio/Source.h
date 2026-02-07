#pragma once

#include "Buffer.h"
#include "Titan/Core.h"
#include "Titan/PCH.h"

namespace Titan
{
    class AudioSource
    {
    public:
        virtual ~AudioSource() = default;

        virtual void Play() = 0;
        virtual void Stop() = 0;
        virtual void SetBuffer(const Ref<AudioBuffer>& buffer) = 0;
        virtual Ref<AudioBuffer> GetBuffer() const = 0;

        virtual void SetPosition(float x, float y, float z) = 0;
        virtual void SetVelocity(float x, float y, float z) = 0;

        static Ref<AudioSource> Create();
    };
} // namespace Titan