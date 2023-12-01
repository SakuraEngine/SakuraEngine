#pragma once
#include "SkrRT/platform/atomic.h"
#include <SkrRT/containers/vector.hpp>

namespace skr 
{

template <typename T>
struct RingBuffer 
{
    template <typename U>
    friend struct ResizableRingBuffer;
public:
    RingBuffer(uint64_t length)
    {
        const auto l = length ? length : 32;
        buffer.resize_zeroed(l);
        skr_atomicu64_store_release(&size, l);
    }
    T add(T value);
    T get(uint64_t index = 0);
    uint64_t get_size() const 
    { 
        const auto sz = skr_atomicu64_load_acquire(&size); 
        return sz;
    }
protected:
    skr::Vector<T> buffer;
    SAtomicU64 head = 0;
    SAtomicU64 size = 0;
};

template <typename T>
T RingBuffer<T>::add(T value) 
{
    const auto currentSize = skr_atomicu64_load_acquire(&size);
    const auto currentHead = skr_atomicu64_load_acquire(&head);
    const auto slot = (currentHead + 1) % currentSize;
    const auto old = buffer[slot];
    buffer[slot] = value;
    skr_atomicu64_store_release(&head, slot);
    return old;
}

template <typename T>
T RingBuffer<T>::get(uint64_t index) 
{
    const auto currentSize = skr_atomicu64_load_acquire(&size);
    const auto currentHead = skr_atomicu64_load_acquire(&head);
    const auto slot = (currentHead + index) % currentSize;
    auto ret = buffer[slot];
    return ret;
}

}