#pragma once

#ifdef TI_PLATFORM_WINDOWS
#ifdef TI_BUILD_DLL
#define TI_API _declspec(dllexport)
#else
#define TI_API _declspec(dllimport)
#endif
#else
#error Titan Engine only supports Windows for now!
#endif

#ifdef TI_BUILD_DEBUG
#ifndef TI_ENABLE_ASSERTS
#define TI_ENABLE_ASSERTS
#endif
#ifndef TI_ENABLE_LOGGING
#define TI_ENABLE_LOGGING
#endif
#endif

#ifdef TI_ENABLE_ASSERTS
#define ASSERT(x, ...)                                       \
    {                                                        \
        if (!(x))                                            \
        {                                                    \
            LOG_ERROR("Assertion Failed: {0}", __VA_ARGS__); \
            __debugbreak();                                  \
        }                                                    \
    }
#define CORE_ASSERT(x, ...)                                       \
    {                                                             \
        if (!(x))                                                 \
        {                                                         \
            LOG_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); \
            __debugbreak();                                       \
        }                                                         \
    }
#else
#define ASSERT(x, ...)
#define CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)