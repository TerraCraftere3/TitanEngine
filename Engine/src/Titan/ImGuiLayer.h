#pragma once

#include "Titan/Core.h"
#include "Titan/Layer.h"
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

        void OnImGuiRender(ImGuiContext* ctx) override
        {
            static bool show = true;
            ImGui::ShowDemoWindow(&show);
        }

    private:
        float m_Time = 0.0f;
    };

} // namespace Titan