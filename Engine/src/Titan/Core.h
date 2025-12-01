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
        #define TI_API
    #endif
    #define TI_PLATFORM_STRING "Windows"
#else
    #error Titan Engine only supports Windows for now!
    #define TI_PLATFORM_STRING "NO PLATFORM SPECIFIED"
#endif

#ifdef TI_BUILD_DEBUG
    #ifndef TI_ENABLE_ASSERTS
        #define TI_ENABLE_ASSERTS
    #endif
    #ifndef TI_ENABLE_LOGGING
        #define TI_ENABLE_LOGGING
    #endif
    #ifdef TI_BUILD_DEBUG_OPTIMIZED
        #define TI_VARIANT_STRING "Debug Optimized"
    #else
        #define TI_VARIANT_STRING "Debug"
    #endif
#elif defined(TI_BUILD_RELEASE)
    #define TI_VARIANT_STRING "Release"
#else
    #define TI_VARIANT_STRING "NO VARIANT SPECIFIED"
#endif

#if defined(_M_X64) || defined(__amd64__) || defined(__x86_64__)
    #define TI_ARCH_X64
    #define TI_ARCH_STRING "x64"
#elif defined(_M_IX86) || defined(__i386__)
    #define TI_ARCH_X86
    #define TI_ARCH_STRING "x86"
#elif defined(_M_ARM) || defined(__arm__)
    #define TI_ARCH_ARM32
    #define TI_ARCH_STRING "ARM32"
#elif defined(_M_ARM64) || defined(__aarch64__)
    #define TI_ARCH_ARM64
    #define TI_ARCH_STRING "ARM64"
#else
    #error Unknown architecture!
    #define TI_ARCH_STRING "Unknown"
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

#undef min
#undef max