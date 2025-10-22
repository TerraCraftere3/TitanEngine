#pragma once
#include <string_view>

constexpr std::string_view extract_func_name(std::string_view sig)
{
    size_t start = sig.rfind(' ') + 1;
    size_t end = sig.find('(', start);
    if (start != std::string_view::npos && end != std::string_view::npos)
    {
        return sig.substr(start, end - start);
    }

    return sig;
}

#ifdef _MSC_VER
#define TI_FUNC_NAME extract_func_name(__FUNCSIG__).data()
#elif defined(__GNUC__) || defined(__clang__)
#define TI_FUNC_NAME extract_func_name(__PRETTY_FUNCTION__).data()
#else
#define TI_FUNC_NAME __func__
#endif