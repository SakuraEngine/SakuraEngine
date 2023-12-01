#pragma once
#include "SkrRT/containers_new/resizable_ring_buffer.hpp"

namespace skr {
namespace input {

template<typename T>
struct ReadingRing : public ResizableRingBuffer<T>
{
    ReadingRing() SKR_NOEXCEPT
        : ResizableRingBuffer<T>(256)
    {

    }
};

} }