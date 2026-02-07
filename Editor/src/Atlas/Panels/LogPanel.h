#pragma once
#include <Titan/PCH.h>

namespace Titan
{
    class LogPanel
    {
    public:
        LogPanel();

        void OnImGuiRender(bool* open = nullptr);

    private:
        bool m_AutoScroll = true;
        bool m_ShowTimestamp = true;
        bool m_ShowOnlyErrors = false;
        int m_SelectedIndex = -1;
    };

} // namespace Titan
