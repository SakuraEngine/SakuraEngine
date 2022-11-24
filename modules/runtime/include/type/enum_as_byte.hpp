#pragma once
#include <type_traits>

namespace skr
{
    template<class T>
    struct TEnumAsByte
    {
        static_assert(std::is_enum<T>::value, "T must be an enum type");
        using UT = std::underlying_type_t<T>;
        T value;
        const auto& as_byte() const
        {
            return (const UT&)(value);
        }
        auto& as_byte()
        {
            return (UT&)(value);
        }
        static TEnumAsByte<T> from_byte(UT v)
        {
            return { static_cast<T>(v) };
        }
        operator T() const
        {
            return value;
        }
    };
}