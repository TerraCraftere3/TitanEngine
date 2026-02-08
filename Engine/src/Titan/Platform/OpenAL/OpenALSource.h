#pragma once
#include "Titan/Audio/Source.h"

namespace Titan
{
    class OpenALSource : public AudioSource
    {
    public:
        OpenALSource();
        ~OpenALSource() override;

        void Play() override;
        void Stop() override;
        void SetBuffer(const Ref<AudioBuffer>& buffer) override;
        Ref<AudioBuffer> GetBuffer() const override;

        void SetPosition(float x, float y, float z) override;
        void SetVelocity(float x, float y, float z) override;
        void SetPitch(float pitch) override;
        void SetGain(float gain) override;
        void SetLooping(bool loop) override;

    private:
        Ref<AudioBuffer> m_Buffer;
        uint32_t m_SourceID = 0;
    };
} // namespace Titan