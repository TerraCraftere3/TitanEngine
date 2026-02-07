#pragma once
#include "Titan/Audio/Listener.h"

namespace Titan
{
    class OpenALListener : public AudioListener
    {
    public:
        OpenALListener();
        ~OpenALListener() override;

        void SetPosition(glm::vec3 position) override;
        void SetOrientation(float atX, float atY, float atZ, float upX, float upY, float upZ) override;

        glm::vec3 GetPosition() override;
        void GetOrientation(float& atX, float& atY, float& atZ, float& upX, float& upY, float& upZ) override;
    };
} // namespace Titan