#pragma once
#include "SkrBase/config.h"
#include "SkrBase/misc/debug.h"
#include "SkrBase/memory/memory_ops.hpp"

namespace skr::container
{
template <typename TSize, typename TP>
inline void process_ring_buffer_data(TSize capacity, TSize front, TSize back, TP&& processor) noexcept
{
    const TSize solved_front = front % capacity;
    const TSize solved_back  = back % capacity;

    if (solved_front < solved_back || (solved_front == 0 && solved_back == 0)) // continuous memory
    {
        processor(0, solved_front, back - front);
    }
    else // broken memory
    {
        // process front part
        const TSize front_part_size = capacity - solved_front;
        processor(0, solved_front, front_part_size);

        // process back part
        processor(front_part_size, 0, solved_back);
    }
}

template <typename T, typename TSize>
inline void copy_ring_buffer(T* dst, const T* src, TSize src_capacity, TSize src_front, TSize src_back) noexcept
{
    SKR_ASSERT(src_back != src_front && src_back - src_front <= src_capacity && "src buffer data is invalid");

    process_ring_buffer_data(
        src_capacity,
        src_front,
        src_back,
        [&dst, &src](TSize dst_offset, TSize src_offset, TSize size) {
            ::skr::memory::copy(dst + dst_offset, src + src_offset, size);
        }
    );
}
template <typename T, typename TSize>
inline void move_ring_buffer(T* dst, T* src, TSize src_capacity, TSize src_front, TSize src_back) noexcept
{
    SKR_ASSERT(src_back != src_front && src_back - src_front <= src_capacity && "src buffer data is invalid");

    process_ring_buffer_data(
        src_capacity,
        src_front,
        src_back,
        [&dst, &src](TSize dst_offset, TSize src_offset, TSize size) {
            ::skr::memory::move(dst + dst_offset, src + src_offset, size);
        }
    );
}
template <typename T, typename TSize>
inline void destruct_ring_buffer(T* buffer, TSize capacity, TSize front, TSize back) noexcept
{
    SKR_ASSERT(back != front && back - front <= capacity && "buffer data is invalid");

    process_ring_buffer_data(
        capacity,
        front,
        back,
        [&buffer](TSize dst_offset, TSize src_offset, TSize size) {
            ::skr::memory::destruct(buffer + src_offset, size);
        }
    );
}
} // namespace skr::container
