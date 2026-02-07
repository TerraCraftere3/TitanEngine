#include "OpenALContext.h"

#include "Titan/PCH.h"

#include <AL/al.h>
#include <AL/alc.h>

namespace Titan
{
    OpenALContext::OpenALContext()
    {
        m_Device = alcOpenDevice(nullptr);
        if (!m_Device)
        {
            TI_CORE_ERROR("Failed to open default OpenAL device");
            return;
        }

        m_Context = alcCreateContext(m_Device, nullptr);
        if (!m_Context)
        {
            TI_CORE_ERROR("Failed to create OpenAL context");
            alcCloseDevice(m_Device);
            m_Device = nullptr;
            return;
        }

        if (alcMakeContextCurrent(m_Context) == ALC_FALSE)
        {
            TI_CORE_ERROR("Failed to make OpenAL context current");
            alcDestroyContext(m_Context);
            alcCloseDevice(m_Device);
            m_Context = nullptr;
            m_Device = nullptr;
            return;
        }
    }

    OpenALContext::~OpenALContext()
    {
        if (m_Context)
        {
            alcMakeContextCurrent(nullptr);
            alcDestroyContext(m_Context);
            m_Context = nullptr;
        }

        if (m_Device)
        {
            alcCloseDevice(m_Device);
            m_Device = nullptr;
        }
    }
} // namespace Titan
