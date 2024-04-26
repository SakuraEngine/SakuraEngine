#pragma once
#include <SkrContainers/string.hpp>
#include <SkrContainers/span.hpp>

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
#ifndef __meta__
        static_assert(std::is_same_v<T, T*>, "EnumTraits<T>::items is not implemented");
#endif
        return {};
    }
    static StringView to_string(const T& value)
    {
#ifndef __meta__
        static_assert(std::is_same_v<T, T*>, "EnumTraits<T>::to_string is not implemented");
#endif
        return {};
    }
    static bool from_string(StringView str, T& value)
    {
#ifndef __meta__
        static_assert(std::is_same_v<T, T*>, "EnumTraits<T>::from_string is not implemented");
#endif
        return false;
    }
};
} // namespace skr::rttr
