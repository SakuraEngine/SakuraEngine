#pragma once
#include "containers/resizable_ring_buffer.hpp"

namespace skr {
namespace input {

template<typename T>
struct ReadingRing : public resizable_ring_buffer<T>
{
    ReadingRing() SKR_NOEXCEPT
        : resizable_ring_buffer<T>(256)
    {

    }
};

} }