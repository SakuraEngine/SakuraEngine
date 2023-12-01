#pragma once
#include "SkrRT/containers_new/stl_deque.hpp"
#include <queue>

namespace skr
{

template<typename T, typename Container = skr::deque<T>>
using queue = std::queue<T, Container>;

} // namespace skr