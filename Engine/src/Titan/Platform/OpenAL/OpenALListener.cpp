#include "OpenALListener.h"

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

    OpenALListener::OpenALListener()
    {
        alGetError();
        alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
        const float orientation[6] = {0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f};
        alListenerfv(AL_ORIENTATION, orientation);
        LogALError("OpenALListener setup");
    }

    OpenALListener::~OpenALListener() = default;

    void OpenALListener::SetPosition(glm::vec3 position)
    {
        alListener3f(AL_POSITION, position.x, position.y, position.z);
        LogALError("alListener3f(AL_POSITION)");
    }

    void OpenALListener::SetOrientation(float atX, float atY, float atZ, float upX, float upY, float upZ)
    {
        const float orientation[6] = {atX, atY, atZ, upX, upY, upZ};
        alListenerfv(AL_ORIENTATION, orientation);
        LogALError("alListenerfv(AL_ORIENTATION)");
    }

    glm::vec3 OpenALListener::GetPosition()
    {
        glm::vec3 position(0.0f);
        alGetListener3f(AL_POSITION, &position.x, &position.y, &position.z);
        LogALError("alGetListener3f(AL_POSITION)");
        return position;
    }

    void OpenALListener::GetOrientation(float& atX, float& atY, float& atZ, float& upX, float& upY, float& upZ)
    {
        float orientation[6] = {};
        alGetListenerfv(AL_ORIENTATION, orientation);
        LogALError("alGetListenerfv(AL_ORIENTATION)");

        atX = orientation[0];
        atY = orientation[1];
        atZ = orientation[2];
        upX = orientation[3];
        upY = orientation[4];
        upZ = orientation[5];
    }
} // namespace Titan
