#include <algorithm>
#include <cctype>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <type_traits>

namespace Titan
{
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
} // namespace Titan