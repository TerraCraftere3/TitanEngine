#pragma once

// clang-format off
#include <Optick.h>

#ifdef TI_PROFILE
    #define TI_PROFILE_BEGIN_SESSION(name, filepath) OPTICK_START_CAPTURE(); static const char* ti_profile_path = filepath
    #define TI_PROFILE_END_SESSION() OPTICK_STOP_CAPTURE(); OPTICK_SAVE_CAPTURE(ti_profile_path)
    #define TI_PROFILE_SCOPE(name) OPTICK_EVENT(name)
    #define TI_PROFILE_FUNCTION() OPTICK_EVENT()
#else
    #define TI_PROFILE_BEGIN_SESSION(name, filepath)
    #define TI_PROFILE_END_SESSION()
    #define TI_PROFILE_SCOPE(name)
    #define TI_PROFILE_FUNCTION()
#endif
// clang-format on