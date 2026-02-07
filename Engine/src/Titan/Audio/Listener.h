#pragma once

#include "Titan/Core.h"
#include "Titan/PCH.h"

namespace Titan
{
    class AudioListener
    {
    public:
        virtual ~AudioListener() = default;

        virtual void SetPosition(glm::vec3 position) = 0;
        virtual void SetOrientation(float atX, float atY, float atZ, float upX, float upY, float upZ) = 0;

        virtual glm::vec3 GetPosition() = 0;
        virtual void GetOrientation(float& atX, float& atY, float& atZ, float& upX, float& upY, float& upZ) = 0;

        static Ref<AudioListener> Create();
    };
} // namespace Titan