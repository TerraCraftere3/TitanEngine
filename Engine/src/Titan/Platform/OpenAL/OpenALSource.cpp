#include "OpenALSource.h"

#include "OpenALBuffer.h"
#include "Titan/PCH.h"

#include <AL/al.h>

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
    } // namespace

    OpenALSource::OpenALSource()
    {
        alGetError();
        alGenSources(1, &m_SourceID);
        LogALError("alGenSources");

        if (m_SourceID == 0)
            return;

        alSourcef(m_SourceID, AL_GAIN, 1.0f);
        alSourcef(m_SourceID, AL_PITCH, 1.0f);
        alSource3f(m_SourceID, AL_POSITION, 0.0f, 0.0f, 0.0f);
        alSource3f(m_SourceID, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
        alSourcei(m_SourceID, AL_LOOPING, AL_FALSE);
        LogALError("alSource setup");
    }

    OpenALSource::~OpenALSource()
    {
        if (m_SourceID != 0)
            alDeleteSources(1, &m_SourceID);
    }

    void OpenALSource::Play()
    {
        if (m_SourceID == 0)
            return;

        alSourcePlay(m_SourceID);
        LogALError("alSourcePlay");
    }

    void OpenALSource::Stop()
    {
        if (m_SourceID == 0)
            return;

        alSourceStop(m_SourceID);
        LogALError("alSourceStop");
    }

    void OpenALSource::SetBuffer(const Ref<AudioBuffer>& buffer)
    {
        if (m_SourceID == 0)
            return;

        if (!buffer)
        {
            alSourcei(m_SourceID, AL_BUFFER, 0);
            LogALError("alSourcei(AL_BUFFER, 0)");
            return;
        }

        auto openALBuffer = std::dynamic_pointer_cast<OpenALBuffer>(buffer);
        if (!openALBuffer)
        {
            TI_CORE_ERROR("Audio buffer is not an OpenAL buffer");
            return;
        }

        alSourcei(m_SourceID, AL_BUFFER, static_cast<ALint>(openALBuffer->GetRendererID()));
        LogALError("alSourcei(AL_BUFFER)");

        m_Buffer = buffer;
    }

    Ref<AudioBuffer> OpenALSource::GetBuffer() const
    {
        return m_Buffer;
    }

    void OpenALSource::SetPosition(float x, float y, float z)
    {
        if (m_SourceID == 0)
            return;

        alSource3f(m_SourceID, AL_POSITION, x, y, z);
        LogALError("alSource3f(AL_POSITION)");
    }

    void OpenALSource::SetVelocity(float x, float y, float z)
    {
        if (m_SourceID == 0)
            return;

        alSource3f(m_SourceID, AL_VELOCITY, x, y, z);
        LogALError("alSource3f(AL_VELOCITY)");
    }
} // namespace Titan
