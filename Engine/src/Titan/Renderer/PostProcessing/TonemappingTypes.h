#pragma once

#include "Titan/PCH.h"

namespace Titan
{
    enum class TonemappingOperator
    {
        None = 0,
        Filmic = 1
    };

    inline const char* TonemappingOperatorToString(TonemappingOperator op)
    {
        switch (op)
        {
            case TonemappingOperator::None:
                return "None";
            case TonemappingOperator::Filmic:
                return "Filmic";
            default:
                return "Unknown";
        }
    }
} // namespace Titan
