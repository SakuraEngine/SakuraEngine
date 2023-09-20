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
    // static Span<EnumItem<T>> items();
    // static string_view       to_string(const T& value);
    // static bool              from_string(string_view str, T& value);
};
} // namespace skr::rttr
