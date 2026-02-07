#pragma once
#include "Titan/Audio/AudioContext.h"

struct ALCdevice;
struct ALCcontext;

namespace Titan
{
    class OpenALContext : public AudioContext
    {
    public:
        OpenALContext();
        ~OpenALContext() override;

    private:
        ALCdevice* m_Device = nullptr;
        ALCcontext* m_Context = nullptr;
    };
} // namespace Titan