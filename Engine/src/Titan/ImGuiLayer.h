#pragma once

#include "Core.h"
#include "Layer.h"
#include "PCH.h"

#include "Events/ApplicationEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"

namespace Titan
{

    class TI_API ImGuiLayer : public Layer
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