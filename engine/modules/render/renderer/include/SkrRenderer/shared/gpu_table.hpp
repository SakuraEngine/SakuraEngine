#pragma once
#include "detail/gpu_table.h"

namespace skr::gpu
{

template<typename T>
inline constexpr bool is_table_range_v = false;
template<typename U>
inline constexpr bool is_table_range_v<Row<U>> = true;
template<typename U>
inline constexpr bool is_table_range_v<Range<U>> = true;
template<typename U, uint32_t N>
inline constexpr bool is_table_range_v<FixedRange<U, N>> = true;

} // skr::gpu