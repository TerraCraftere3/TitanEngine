#pragma once

#include "Titan/Core.h"
#include "Titan/PCH.h"

namespace Titan
{
    class AudioBuffer
    {
    public:
        virtual ~AudioBuffer() = default;

        virtual uint32_t GetSampleRate() const = 0;
        virtual uint32_t GetChannelCount() const = 0;
        virtual uint64_t GetFrameCount() const = 0;
        virtual float GetDuration() const = 0;
        virtual const std::vector<float>& GetAudioData() const = 0;

        void SetInternalPath(const std::string& path) { m_InternalPath = path; }
        std::string GetInternalPath() const { return m_InternalPath; }

        static Ref<AudioBuffer> Create(const std::filesystem::path& filepath);

    protected:
        std::string m_InternalPath;
    };
} // namespace Titan