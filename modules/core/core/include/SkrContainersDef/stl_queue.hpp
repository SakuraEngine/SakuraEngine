#pragma once
#include "SkrContainersDef/stl_deque.hpp"
#include <queue>

namespace skr
{

template <typename T, typename Container = skr::stl_deque<T>>
using queue = std::queue<T, Container>;

} // namespace skr