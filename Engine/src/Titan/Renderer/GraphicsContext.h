#pragma once

#include "Titan/Core.h"
#include "Titan/PCH.h"

namespace Titan
{

    class GraphicsContext
    {
    public:
        virtual void Init() = 0;
        virtual void Swapbuffers() = 0;
    };

} // namespace Titan