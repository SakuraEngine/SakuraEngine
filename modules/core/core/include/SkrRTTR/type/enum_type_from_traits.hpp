#pragma once
#include "SkrRTTR/type/enum_type.hpp"
#include "SkrRTTR/enum_traits.hpp"
#include "SkrRTTR/rttr_traits.hpp"

namespace skr::rttr
{
template <typename T>
struct EnumTypeFromTraits : public EnumType {
    EnumTypeFromTraits()
        : EnumType(type_of<std::underlying_type_t<T>>(), type_id_of<T>(), type_name_of<T>())
    {
    }

    EnumValue value_from_string(StringView str) const override
    {
        T result;
        if (EnumTraits<T>::from_string(str, result))
        {
            return EnumValue(static_cast<std::underlying_type_t<T>>(result));
        }
        else
        {
            return {};
        }
    }
    String value_to_string(const EnumValue& value) const override
    {
        T result;
        if (value.cast_to(result))
        {
            return EnumTraits<T>::to_string(result);
        }
        else
        {
            return u8"";
        }
    }
};
} // namespace skr::rttr