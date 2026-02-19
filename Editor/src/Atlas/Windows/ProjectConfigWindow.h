#pragma once

#include "Titan/Core.h"
#include "Titan/Core/Log.h"

namespace Titan
{

    class ProjectConfigWindow
    {
    public:
        ProjectConfigWindow() = default;

        void OnImGuiRender();
        bool* GetOpenPtr() { return &m_Open; }

    private:
        bool m_Open = false;
    };

} // namespace Titan