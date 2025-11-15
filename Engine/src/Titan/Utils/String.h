#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <type_traits>

namespace Titan{
    template<typename T>
std::string FormatNumber(T value) {
    static_assert(std::is_arithmetic<T>::value, "Value must be numeric");

    // Remove fractional part for floating-point numbers
    if constexpr (std::is_floating_point<T>::value) {
        value = static_cast<long long>(value);
    }

    std::ostringstream oss;
    oss.imbue(std::locale("")); // Uses the user's locale for thousands separator
    oss << value;
    return oss.str();
}
}