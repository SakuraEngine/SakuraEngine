#pragma once
#include <array>

namespace skr
{
template <class T, size_t Size>
using Array = std::array<T, Size>;

namespace arrayDetail
{
template <typename T, std::size_t... Is>
constexpr Array<T, sizeof...(Is)>
create_array(T value, std::index_sequence<Is...>)
{
    // cast Is to void to remove the warning: unused value
    return { { (static_cast<void>(Is), value)... } };
}
} // namespace arrayDetail

template <typename T, std::size_t N>
constexpr Array<T, N> create_array(T&& value)
{
    return arrayDetail::create_array(std::forward<T>(value), std::make_index_sequence<N>());
}
} // namespace skr