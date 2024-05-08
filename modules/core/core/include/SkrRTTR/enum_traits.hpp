#pragma once
#include <SkrContainers/string.hpp>
#include <SkrContainers/span.hpp>
#include "SkrBase/meta.h"

namespace skr::rttr
{
template <class T>
struct EnumItem {
    StringView name  = {};
    T          value = {};
};

template <class T>
struct EnumTraits {
    static span<EnumItem<T>> items()
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
} // namespace skr::rttr
