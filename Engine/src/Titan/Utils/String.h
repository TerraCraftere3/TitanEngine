#pragma once
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>

namespace Titan
{
    /// @brief Formats a number with thousands separators according to the user's locale
    /// @tparam T  The numeric type of the value
    /// @param value the numeric value to format
    /// @return the formatted string
    template <typename T>
    std::string FormatNumber(T value)
    {
        static_assert(std::is_arithmetic<T>::value, "Value must be numeric");

        // Remove fractional part for floating-point numbers
        if constexpr (std::is_floating_point<T>::value)
        {
            value = static_cast<long long>(value);
        }

        std::ostringstream oss;
        oss.imbue(std::locale("")); // Uses the user's locale for thousands separator
        oss << value;
        return oss.str();
    }

    /// @brief Removes invalid characters from a string to make it safe for file names (replaces with '_')
    /// @param name the input name
    /// @return the sanitized name
    inline std::string SanitizeName(const std::string& name)
    {
        std::string result = name;
        std::replace_if(
            result.begin(), result.end(),
            [](char c) { return !std::isalnum(static_cast<unsigned char>(c)) && c != '-' && c != '_'; }, '_');
        return result;
    }

    /// @brief Truncates a string to a maximum length, adding ellipsis if truncated
    /// @param input the input string
    /// @param maxLen the length limit
    /// @return the truncated string
    inline std::string TruncateString(const std::string& input, std::size_t maxLen)
    {
        const std::string ellipsis = "...";
        if (input.size() <= maxLen || maxLen <= ellipsis.size())
            return input;

        return input.substr(0, maxLen - ellipsis.size()) + ellipsis;
    }
} // namespace Titan