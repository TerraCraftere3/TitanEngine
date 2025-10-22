#pragma once

#include <memory>

// clang-format off
#ifdef TI_PLATFORM_WINDOWS
    #ifdef TI_DYNAMIC_LINK
        #ifdef TI_BUILD_DLL
            #define TI_API _declspec(dllexport)
        #else
            #define TI_API _declspec(dllimport)
        #endif
    #else
        
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
#define TI_ASSERT(x, ...)                                    \
    {                                                        \
        if (!(x))                                            \
        {                                                    \
            TI_ERROR("Assertion Failed: " __VA_ARGS__); \
            __debugbreak();                                  \
        }                                                    \
    }
#define TI_CORE_ASSERT(x, ...)                                    \
    {                                                             \
        if (!(x))                                                 \
        {                                                         \
            TI_CORE_ERROR("Assertion Failed: " __VA_ARGS__); \
            __debugbreak();                                       \
        }                                                         \
    }
#else
    #define TI_ASSERT(x, ...)
    #define TI_CORE_ASSERT(x, ...)
#endif

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#define BIT(x) (1 << x)
#define TI_BIND_EVENT_FN(fn) std::bind(&##fn, this, std::placeholders::_1)

#define TI_CONCAT_INNER(a, b) a##b
#define TI_CONCAT(a, b) TI_CONCAT_INNER(a, b)
// clang-format on

namespace Titan
{

    template <typename T>
    using Scope = std::unique_ptr<T>;
    template <typename T, typename... Args>
    constexpr Scope<T> CreateScope(Args&&... args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template <typename T>
    using Ref = std::shared_ptr<T>;
    template <typename T, typename... Args>
    constexpr Ref<T> CreateRef(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

} // namespace Titan