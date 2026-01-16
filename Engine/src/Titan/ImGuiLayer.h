#pragma once

#include "Titan/Core.h"
#include "Titan/Core/Layer.h"
#include "Titan/PCH.h"

#include "Titan/Events/ApplicationEvent.h"
#include "Titan/Events/KeyEvent.h"
#include "Titan/Events/MouseEvent.h"

namespace Titan
{
    enum class UITheme
    {
        Dark,
        OLED
    };

    class ImGuiLayer : public Layer
    {
    public:
        ImGuiLayer(UITheme theme = UITheme::Dark);
        ~ImGuiLayer();

        virtual void OnAttach() override;
        virtual void OnDetach() override;

        void Begin();
        void End();

        void SetTheme(UITheme theme);

    private:
        void SetupOLEDStyles();
        void SetupDarkStyles();

    private:
        UITheme m_Theme;
        float m_Time = 0.0f;
        std::string m_ImGuiConfigPath;
    };

} // namespace Titan