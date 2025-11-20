#pragma once

#include <type_traits>
#include <utility>
#include "Titan/PCH.h"
#include <entt/entt.hpp>

namespace Titan::EnttCompat
{
    // Detection helpers
    template <typename R, typename T, typename = void>
    struct has_any_of : std::false_type {};

    template <typename R, typename T>
    struct has_any_of<R, T, std::void_t<decltype(std::declval<R>().template any_of<T>(std::declval<entt::entity>()))>> : std::true_type
    {
    };

    template <typename R, typename T, typename = void>
    struct has_all_of : std::false_type {};

    template <typename R, typename T>
    struct has_all_of<R, T, std::void_t<decltype(std::declval<R>().template all_of<T>(std::declval<entt::entity>()))>> : std::true_type
    {
    };

    template <typename R, typename T, typename = void>
    struct has_has : std::false_type {};

    template <typename R, typename T>
    struct has_has<R, T, std::void_t<decltype(std::declval<R>().template has<T>(std::declval<entt::entity>()))>> : std::true_type
    {
    };

    template <typename R, typename T, typename = void>
    struct has_try_get : std::false_type {};

    template <typename R, typename T>
    struct has_try_get<R, T, std::void_t<decltype(std::declval<R>().template try_get<T>(std::declval<entt::entity>()))>> : std::true_type
    {
    };

    template <typename R, typename T, typename = void>
    struct has_get : std::false_type {};

    template <typename R, typename T>
    struct has_get<R, T, std::void_t<decltype(std::declval<R>().template get<T>(std::declval<entt::entity>()))>> : std::true_type
    {
    };

    // registry_has: checks whether entity has component T using whichever API is available
    template <typename R, typename T>
    inline bool registry_has(R& reg, entt::entity e)
    {
        if constexpr (has_any_of<R, T>::value)
            return reg.template any_of<T>(e);
        else if constexpr (has_all_of<R, T>::value)
            return reg.template all_of<T>(e);
        else if constexpr (has_has<R, T>::value)
            return reg.template has<T>(e);
        else if constexpr (has_try_get<R, T>::value)
            return reg.template try_get<T>(e) != nullptr;
        else
            static_assert(has_any_of<R, T>::value || has_all_of<R, T>::value || has_has<R, T>::value || has_try_get<R, T>::value,
                          "Entt registry has no known component-test API");
    }

    // registry_get: returns reference to component T. If only try_get is available it will assert on null.
    template <typename R, typename T>
    inline T& registry_get(R& reg, entt::entity e)
    {
        if constexpr (has_get<R, T>::value)
            return reg.template get<T>(e);
        else if constexpr (has_try_get<R, T>::value)
        {
            auto ptr = reg.template try_get<T>(e);
            TI_CORE_ASSERT(ptr, "registry_get: component not present");
            return *ptr;
        }
        else
            static_assert(has_get<R, T>::value || has_try_get<R, T>::value, "Entt registry has no known get API");
    }

} // namespace Titan::EnttCompat
