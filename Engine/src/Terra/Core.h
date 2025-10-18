#pragma once

#ifdef TERRA_PLATFORM_WINDOWS
#ifdef TERRA_BUILD_DLL
#define TERRA_API _declspec(dllexport)
#else
#define TERRA_API _declspec(dllimport)
#endif
#else
#error Terra Engine only supports Windows for now!
#endif

#ifdef TERRA_BUILD_DEBUG
#ifndef TERRA_ENABLE_ASSERTS
#define TERRA_ENABLE_ASSERTS
#endif
#endif

#ifdef TERRA_ENABLE_ASSERTS
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
#define HZ_ASSERT(x, ...)
#define HZ_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)