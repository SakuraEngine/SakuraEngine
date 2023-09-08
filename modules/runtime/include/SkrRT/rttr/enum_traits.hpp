#pragma once
#include <SkrRT/containers/string.hpp>

namespace skr::rttr
{
template <class T>
struct EnumTraits {
    static string_view to_string(const T& value);
    static bool        from_string(string_view str, T& value);
};
} // namespace skr::rttr
