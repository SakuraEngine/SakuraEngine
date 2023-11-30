#pragma once
#include <SkrRT/containers/string.hpp>
#include <SkrRT/containers_new/span.hpp>

namespace skr::rttr
{
template <class T>
struct EnumItem {
    string_view name  = {};
    T           value = {};
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
    static string_view to_string(const T& value)
    {
#ifndef __meta__
        static_assert(std::is_same_v<T, T*>, "EnumTraits<T>::to_string is not implemented");
#endif
        return {};
    }
    static bool from_string(string_view str, T& value)
    {
#ifndef __meta__
        static_assert(std::is_same_v<T, T*>, "EnumTraits<T>::from_string is not implemented");
#endif
        return false;
    }
};
} // namespace skr::rttr
