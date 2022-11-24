#pragma once
#include <type_traits>
#include "platform/configure.h"

namespace skr
{
    template<class T>
    struct TEnumAsByte
    {
        static_assert(std::is_enum<T>::value, "T must be an enum type");
        using UT = std::underlying_type_t<T>;
        inline TEnumAsByte() SKR_NOEXCEPT = default;
        inline TEnumAsByte(T enumeration) SKR_NOEXCEPT
            : value(enumeration)
        {

        }
        inline TEnumAsByte<T>& operator=(T enumeration) SKR_NOEXCEPT
        {
            this->value = enumeration;
            return *this;
        }
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