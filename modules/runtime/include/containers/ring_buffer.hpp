#pragma once
#include "platform/atomic.h"
#include <containers/vector.hpp>

namespace skr 
{

template <typename T>
struct ring_buffer 
{
    template <typename U>
    friend struct resizable_ring_buffer;
public:
    ring_buffer(uint64_t length)
    {
        const auto l = length ? length : 32;
        buffer.resize(l);
        skr_atomic64_store_release(&size, l);
    }
    T add(T value);
    T get(uint64_t index = 0);
    uint64_t get_size() const 
    { 
        const auto sz = skr_atomic64_load_acquire(&size); 
        return sz;
    }
protected:
    skr::vector<T> buffer;
    SAtomic64 head = 0;
    SAtomic64 size = 0;
};

template <typename T>
T ring_buffer<T>::add(T value) 
{
    const auto currentSize = skr_atomic64_load_acquire(&size);
    const auto currentHead = skr_atomic64_load_acquire(&head);
    const auto slot = (currentHead + 1) % currentSize;
    const auto old = buffer[slot];
    buffer[slot] = value;
    skr_atomic64_store_release(&head, slot);
    return old;
}

template <typename T>
T ring_buffer<T>::get(uint64_t index) 
{
    const auto currentSize = skr_atomic64_load_acquire(&size);
    const auto currentHead = skr_atomic64_load_acquire(&head);
    const auto slot = (currentHead + index) % currentSize;
    auto ret = buffer[slot];
    return ret;
}

}