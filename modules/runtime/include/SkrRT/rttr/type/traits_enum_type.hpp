#pragma once
#include "SkrRT/rttr/type/enum_type.hpp"
#include "SkrRT/rttr/enum_traits.hpp"
#include "SkrRT/rttr/rttr_traits.hpp"

namespace skr::rttr
{
template <typename T>
struct TraitsEnumType : public EnumType {
    TraitsEnumType()
        : EnumType(get_underlying_type<T>(), RTTRTraits<T>::get_guid())
    {
    }

    EnumValue value_from_string(string_view str) const
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
    string value_to_string(const EnumValue& value) const
    {
        T result;
        if (value.cast_to(result))
        {
            return EnumTraits<T>::to_string(result);
        }
        else
        {
            return "";
        }
    }
};
} // namespace skr::rttr