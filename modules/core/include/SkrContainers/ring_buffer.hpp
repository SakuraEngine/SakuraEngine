#pragma once
#include "SkrOS/atomic.h"
#include "SkrOS/thread.h"
#include <SkrContainers/vector.hpp>
#include "SkrBase/containers/ring_buffer/ring_buffer.hpp"
#include "SkrBase/containers/ring_buffer/ring_buffer_memory.hpp"

namespace skr
{
template <typename T, typename Allocator = SkrAllocator>
using RingBuffer = container::RingBuffer<container::RingBufferMemory<
T,        /*type*/
uint64_t, /*size type*/
Allocator /*allocator type*/
>>;

template <typename T, uint64_t kCount>
using FixedRingBuffer = container::RingBuffer<container::FixedRingBufferMemory<
T,        /*type*/
uint64_t, /*size type*/
kCount    /*count*/
>>;

template <typename T, uint64_t kInlineCount, typename Allocator = SkrAllocator>
using InlineRingBuffer = container::RingBuffer<container::InlineRingBufferMemory<
T,            /*type*/
uint64_t,     /*size type*/
kInlineCount, /*inline count*/
Allocator     /*allocator type*/
>>;
} // namespace skr

namespace skr
{
template <typename T>
struct SimpleThreadSafeRingBuffer {
    inline SimpleThreadSafeRingBuffer(uint64_t length = 256)
    {
        // resize
        length = length ? length : 32;
        _buffer.resize_zeroed(length);
        skr_atomicu64_store_release(&_size, length);

        skr_init_rw_mutex(&_rw_mutex);
    }
    inline ~SimpleThreadSafeRingBuffer()
    {
        skr_destroy_rw_mutex(&_rw_mutex);
    }

    inline void resize(uint64_t new_size)
    {
        const auto cur_size = skr_atomicu64_load_acquire(&_size);
        if (new_size == cur_size) return;

        skr_rw_mutex_acquire_w(&_rw_mutex);
        if (new_size < cur_size)
        {
            // if the new size is smaller than the current size, we need to
            // copy size - newSize elements from the end of the buffer to the
            // beginning of the buffer
            int offset = cur_size - new_size;
            for (int i = 0; i < new_size; i++)
            {
                _buffer[i] = _buffer[i + offset];
            }
            skr_atomicu64_store_release(&_head, new_size);
        }
        else
        {
            // if the new size is larger than the current size, we need to
            // copy size elements from the beginning of the buffer to the
            // end of the buffer
            _buffer.resize(new_size);
            for (int i = cur_size; i < new_size; i++)
            {
                _buffer[i] = _buffer[i - cur_size];
            }
            skr_atomicu64_store_release(&_head, cur_size);
        }
        skr_atomicu64_store_release(&_size, new_size);
        skr_rw_mutex_release_w(&_rw_mutex);
    }
    inline void zero()
    {
        skr_rw_mutex_acquire_w(&_rw_mutex);
        for (int i = 0; i < _buffer.size(); i++)
        {
            _buffer[i] = {};
        }
        skr_rw_mutex_release_w(&_rw_mutex);
    }
    inline T add(T value)
    {
        skr_rw_mutex_acquire_r(&_rw_mutex);
        const auto cur_size = skr_atomicu64_load_acquire(&_size);
        const auto cur_head = skr_atomicu64_load_acquire(&_head);
        const auto slot     = (cur_head + 1) % cur_size;
        const auto old      = _buffer[slot];
        _buffer[slot]       = value;
        skr_atomicu64_store_release(&_head, slot);
        skr_rw_mutex_release_r(&_rw_mutex);
        return old;
    }
    inline T get(uint64_t index = 0)
    {
        skr_rw_mutex_acquire_r(&_rw_mutex);
        const auto cur_size = skr_atomicu64_load_acquire(&_size);
        const auto cur_head = skr_atomicu64_load_acquire(&_head);
        const auto slot     = (cur_head + index) % cur_size;
        auto       result   = _buffer[slot];
        skr_rw_mutex_release_r(&_rw_mutex);
        return result;
    }
    inline uint64_t get_size() const
    {
        skr_rw_mutex_acquire_r(&_rw_mutex);
        const auto result = skr_atomicu64_load_acquire(&_size);
        skr_rw_mutex_release_r(&_rw_mutex);
        return result;
    }
    inline void acquire_read()
    {
        skr_rw_mutex_acquire_r(&_rw_mutex);
    }
    inline void release_read()
    {
        skr_rw_mutex_release_r(&_rw_mutex);
    }

private:
    Vector<T>        _buffer = {};
    SAtomicU64       _head   = 0;
    SAtomicU64       _size   = 0;
    mutable SRWMutex _rw_mutex;
};
} // namespace skr