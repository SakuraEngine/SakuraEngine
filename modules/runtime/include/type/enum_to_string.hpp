#pragma once
#include "platform/configure.h"
#include <string_view>

namespace skr::type
{
    template<class T>
    struct EnumToStringTrait;

    template<class T>
    std::string_view enum_to_string(const T& value)
    {
        return EnumToStringTrait<T>::ToString(value);
    }

    template<class T>
    bool enum_from_string(std::string_view str, T& value)
    {
        return EnumToStringTrait<T>::FromString(str, value);
    }
}