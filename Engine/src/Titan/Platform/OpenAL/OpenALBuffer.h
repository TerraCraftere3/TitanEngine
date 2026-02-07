#pragma once
#include "Titan/Audio/Buffer.h"

namespace Titan
{
    class OpenALBuffer : public AudioBuffer
    {
    public:
        OpenALBuffer(const std::filesystem::path& filepath);
        ~OpenALBuffer() override;

        uint32_t GetSampleRate() const override;
        uint32_t GetChannelCount() const override;
        uint64_t GetFrameCount() const override;
        float GetDuration() const override;
        const std::vector<float>& GetAudioData() const override;

        uint32_t GetRendererID() const;

    private:
        uint32_t m_BufferID = 0;
        uint32_t m_SampleRate = 0;
        uint32_t m_ChannelCount = 0;
        uint64_t m_FrameCount = 0;
        float m_Duration = 0.0f;
        std::vector<float> m_AudioData;
    };
} // namespace Titan