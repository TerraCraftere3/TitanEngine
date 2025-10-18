#pragma once

#include "Layer.h"
#include "PCH.h"

namespace Titan
{

    class TI_API ImGuiLayer : public Layer
    {
    public:
        ImGuiLayer();
        ~ImGuiLayer();

        void OnAttach();
        void OnDetach();
        void OnUpdate();
        void OnEvent(Event& event);

    private:
        float m_Time = 0.0f;
    };

} // namespace Titan