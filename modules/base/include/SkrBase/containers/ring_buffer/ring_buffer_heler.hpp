#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/fwd_container.hpp"
#include "SkrBase/misc/debug.h"
#include "SkrBase/memory/memory_ops.hpp"

namespace skr::container
{
template <typename TS, typename TP>
inline void process_ring_buffer_data(TS capacity, TS front, TS back, TP&& processor) noexcept
{
    const TS solved_front = front % capacity;
    const TS solved_back  = back % capacity;

    if (solved_front < solved_back || (solved_front == 0 && solved_back == 0)) // continuous memory
    {
        processor(0, solved_front, back - front);
    }
    else // broken memory
    {
        // process front part
        const TS front_part_size = capacity - solved_front;
        processor(0, solved_front, front_part_size);

        // process back part
        processor(front_part_size, 0, solved_back);
    }
}

template <typename T, typename TS>
inline void copy_ring_buffer(T* dst, const T* src, TS src_capacity, TS src_front, TS src_back) noexcept
{
    SKR_ASSERT(src_back != src_front && src_back - src_front <= src_capacity && "src buffer data is invalid");

    process_ring_buffer_data(
    src_capacity,
    src_front,
    src_back,
    [&dst, &src](TS dst_offset, TS src_offset, TS size) {
        memory::copy(dst + dst_offset, src + src_offset, size);
    });
}
template <typename T, typename TS>
inline void move_ring_buffer(T* dst, T* src, TS src_capacity, TS src_front, TS src_back) noexcept
{
    SKR_ASSERT(src_back != src_front && src_back - src_front <= src_capacity && "src buffer data is invalid");

    process_ring_buffer_data(
    src_capacity,
    src_front,
    src_back,
    [&dst, &src](TS dst_offset, TS src_offset, TS size) {
        memory::move(dst + dst_offset, src + src_offset, size);
    });
}
// TODO. 这里发生了越界访问，trim 只是在 realloc 之前的修正，realloc 后应当再次修正
template <typename T, typename TS>
inline void trim_ring_buffer_for_new_capacity(T* buffer, TS capacity, TS new_capacity, TS& front, TS& back) noexcept
{
    SKR_ASSERT(back != front && back - front <= capacity && "buffer data is invalid");

    const TS solved_front = front % capacity;
    const TS solved_back  = back % capacity;

    if (solved_front < solved_back || (solved_front == 0 && solved_back == 0)) // continuous memory
    {
        if (solved_back > new_capacity)
        {
            const TS total_size = back - front;
            memory::move(buffer, buffer + solved_front, total_size);
            front = 0;
            back  = total_size;
        }
        else
        {
            front = solved_front;
            back  = solved_back;
        }
    }
    else // broken memory
    {
        // move front part (in the back of buffer)
        const TS front_part_size  = capacity - solved_front;
        const TS new_solved_front = new_capacity - front_part_size;
        memory::move(buffer + new_solved_front, buffer + solved_front, front_part_size);
        front = new_solved_front;
        back  = solved_back;
    }
}
template <typename T, typename TS>
inline void destruct_ring_buffer(T* buffer, TS capacity, TS front, TS back) noexcept
{
    SKR_ASSERT(back != front && back - front <= capacity && "buffer data is invalid");

    process_ring_buffer_data(
    capacity,
    front,
    back,
    [&buffer](TS dst_offset, TS src_offset, TS size) {
        memory::destruct(buffer + src_offset, size);
    });
}
} // namespace skr::container
