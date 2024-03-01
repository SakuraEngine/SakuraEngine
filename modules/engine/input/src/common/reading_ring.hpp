#pragma once
#include "SkrContainers/ring_buffer.hpp"

namespace skr
{
namespace input
{
template <typename T>
using ReadingRing = SimpleThreadSafeRingBuffer<T>;
} // namespace input
} // namespace skr