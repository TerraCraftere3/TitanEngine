#pragma once

#include <optick.h>
/* =======================
   Macros
   ======================= */

#if 1
#define TI_PROFILE_BEGIN_FRAME() OPTICK_FRAME("Main Thread")
#define TI_PROFILE_END_FRAME()
#define TI_PROFILE_ADD_VARIABLE(name, value) OPTICK_TAG(name, value)
#define TI_PROFILE_SCOPE(name) OPTICK_EVENT(name)
#define TI_PROFILE_FUNCTION() TI_PROFILE_SCOPE(__FUNCTION__)
#else
#define TI_PROFILE_BEGIN_FRAME()
#define TI_PROFILE_END_FRAME()
#define TI_PROFILE_ADD_VARIABLE(name, value)
#define TI_PROFILE_SCOPE(name)
#define TI_PROFILE_FUNCTION()
#endif