#include "OpenALBuffer.h"

#include "Titan/PCH.h"

#include <AL/al.h>
#include <AL/alext.h>
#include <sndfile.h>

namespace Titan
{
    namespace
    {
        const char* GetALErrorString(ALenum error)
        {
            switch (error)
            {
                case AL_NO_ERROR:
                    return "AL_NO_ERROR";
                case AL_INVALID_NAME:
                    return "AL_INVALID_NAME";
                case AL_INVALID_ENUM:
                    return "AL_INVALID_ENUM";
                case AL_INVALID_VALUE:
                    return "AL_INVALID_VALUE";
                case AL_INVALID_OPERATION:
                    return "AL_INVALID_OPERATION";
                case AL_OUT_OF_MEMORY:
                    return "AL_OUT_OF_MEMORY";
                default:
                    return "AL_UNKNOWN_ERROR";
            }
        }

        void LogALError(const char* context)
        {
            ALenum error = alGetError();
            if (error != AL_NO_ERROR)
                TI_CORE_ERROR("OpenAL error {} in {}", GetALErrorString(error), context);
        }

        ALenum GetOpenALFormat(uint32_t channels)
        {
            switch (channels)
            {
                case 1:
                    return AL_FORMAT_MONO_FLOAT32;
                case 2:
                    return AL_FORMAT_STEREO_FLOAT32;
                default:
                    return AL_NONE;
            }
        }
    } // namespace

    OpenALBuffer::OpenALBuffer(const std::filesystem::path& filepath)
    {
        SF_INFO info{};
        SNDFILE* file = nullptr;

#ifdef TI_PLATFORM_WINDOWS
        std::wstring widePath = filepath.wstring();
        file = sf_wchar_open(widePath.c_str(), SFM_READ, &info);
#else
        file = sf_open(filepath.string().c_str(), SFM_READ, &info);
#endif

        if (!file)
        {
            TI_CORE_ERROR("Failed to open audio file '{}': {}", filepath.string(), sf_strerror(nullptr));
            return;
        }

        if (info.channels <= 0 || info.samplerate <= 0 || info.frames <= 0)
        {
            TI_CORE_ERROR("Invalid audio metadata for '{}'", filepath.string());
            sf_close(file);
            return;
        }

        m_SampleRate = static_cast<uint32_t>(info.samplerate);
        m_ChannelCount = static_cast<uint32_t>(info.channels);
        m_FrameCount = static_cast<uint64_t>(info.frames);

        m_AudioData.resize(m_FrameCount * m_ChannelCount);
        sf_count_t framesRead = sf_readf_float(file, m_AudioData.data(), info.frames);
        sf_close(file);

        if (framesRead <= 0)
        {
            TI_CORE_ERROR("Failed to read audio data from '{}'", filepath.string());
            m_AudioData.clear();
            return;
        }

        if (static_cast<uint64_t>(framesRead) != m_FrameCount)
        {
            m_FrameCount = static_cast<uint64_t>(framesRead);
            m_AudioData.resize(m_FrameCount * m_ChannelCount);
        }

        m_Duration = static_cast<float>(m_FrameCount) / static_cast<float>(m_SampleRate);

        if (!alIsExtensionPresent("AL_EXT_FLOAT32"))
        {
            TI_CORE_ERROR("OpenAL float32 extension not available for '{}'", filepath.string());
            return;
        }

        ALenum format = GetOpenALFormat(m_ChannelCount);
        if (format == AL_NONE)
        {
            TI_CORE_ERROR("Unsupported channel count {} for '{}'", m_ChannelCount, filepath.string());
            return;
        }

        alGetError();
        alGenBuffers(1, &m_BufferID);
        LogALError("alGenBuffers");

        if (m_BufferID == 0)
            return;

        alBufferData(m_BufferID, format, m_AudioData.data(), static_cast<ALsizei>(m_AudioData.size() * sizeof(float)),
                     static_cast<ALsizei>(m_SampleRate));
        LogALError("alBufferData");
    }

    OpenALBuffer::~OpenALBuffer()
    {
        if (m_BufferID != 0)
            alDeleteBuffers(1, &m_BufferID);
    }

    uint32_t OpenALBuffer::GetSampleRate() const
    {
        return m_SampleRate;
    }

    uint32_t OpenALBuffer::GetChannelCount() const
    {
        return m_ChannelCount;
    }

    uint64_t OpenALBuffer::GetFrameCount() const
    {
        return m_FrameCount;
    }

    float OpenALBuffer::GetDuration() const
    {
        return m_Duration;
    }

    const std::vector<float>& OpenALBuffer::GetAudioData() const
    {
        return m_AudioData;
    }

    uint32_t OpenALBuffer::GetRendererID() const
    {
        return m_BufferID;
    }
} // namespace Titan
