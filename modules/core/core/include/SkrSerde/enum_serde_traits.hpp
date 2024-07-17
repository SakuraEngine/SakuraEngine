#pragma once
#include <SkrContainersDef/string.hpp>
#include <SkrContainersDef/span.hpp>
#include "SkrBase/meta.h"

// TODO. 仅供 generated.cpp 内使用，退化成静态函数
namespace skr
{
template <typename T>
struct EnumSerdeItem {
    StringView name  = {};
    T          value = {};
};

template <class T>
struct EnumSerdeTraits {
    static span<EnumSerdeItem<T>> items()
    {
        unimplemented_no_meta(T, "EnumTraits<T>::items is not implemented");
        return {};
    }
    static StringView to_string(const T& value)
    {
        unimplemented_no_meta(T, "EnumTraits<T>::to_string is not implemented");
        return {};
    }
    static bool from_string(StringView str, T& value)
    {
        unimplemented_no_meta(T, "EnumTraits<T>::from_string is not implemented");
        return false;
    }
};

} // namespace skr