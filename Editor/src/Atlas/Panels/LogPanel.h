#pragma once
#include <Titan/Core/ImGuiLogSink.h>
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
    };

} // namespace Titan
