#pragma once

#include "Titan/Core.h"
#include "Titan/Core/Layer.h"
#include "Titan/PCH.h"

#include "Titan/Events/ApplicationEvent.h"
#include "Titan/Events/KeyEvent.h"
#include "Titan/Events/MouseEvent.h"

namespace Titan
{

    class ImGuiLayer : public Layer
    {
    public:
        ImGuiLayer();
        ~ImGuiLayer();

        virtual void OnAttach() override;
        virtual void OnDetach() override;

        void Begin();
        void End();

    private:
        float m_Time = 0.0f;
    };

} // namespace Titan