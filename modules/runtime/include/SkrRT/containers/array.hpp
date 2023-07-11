#pragma once
#include <EASTL/array.h>

namespace skr
{
using eastl::array;
namespace arrayDetail
{
template <typename T, std::size_t... Is>
constexpr eastl::array<T, sizeof...(Is)>
create_array(T value, eastl::index_sequence<Is...>)
{
    // cast Is to void to remove the warning: unused value
    return { { (static_cast<void>(Is), value)... } };
}
} // namespace arrayDetail

template <typename T, std::size_t N>
constexpr eastl::array<T, N> create_array(T&& value)
{
    return arrayDetail::create_array(eastl::forward<T>(value), eastl::make_index_sequence<N>());
}
} // namespace skr