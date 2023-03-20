#pragma once
#include <containers/ring_buffer.hpp>
#include "platform/thread.h"

namespace skr 
{

template <typename T>
struct resizable_ring_buffer
{
public:
    resizable_ring_buffer(uint64_t length)
        : ring(length)
    {
        skr_init_mutex_rw(&rw_mutex);
    }
    ~resizable_ring_buffer()
    {
        skr_destroy_rw_mutex(&rw_mutex);
    }
    void resize(int newSize);
    T add(T value);
    T get(uint64_t index = 0);
    uint64_t get_size() const 
    { 
        skr_acquire_mutex_r(&rw_mutex);
        uint64_t sz = ring.get_size();
        skr_release_rw_mutex(&rw_mutex);
        return sz;
    }
    void acquire_read() { skr_acquire_mutex_r(&rw_mutex); }
    void release_read() { skr_release_rw_mutex(&rw_mutex); }
protected:
    ring_buffer<T> ring;
    mutable SRWMutex rw_mutex;
};

template <typename T>
void resizable_ring_buffer<T>::resize(int newSize) 
{
    const auto currentSize = skr_atomic64_load_acquire(&ring.size);
    if (newSize == currentSize) return;

    skr_acquire_mutex_w(&rw_mutex);
    if (newSize < currentSize) 
    {
        // if the new size is smaller than the current size, we need to
        // copy size - newSize elements from the end of the buffer to the
        // beginning of the buffer
        int offset = currentSize - newSize;
        for (int i = 0; i < newSize; i++) 
        {
            ring.buffer[i] = ring.buffer[i + offset];
        }
        skr_atomic32_store_release(&ring.head, newSize);
    } 
    else 
    {
        // if the new size is larger than the current size, we need to
        // copy size elements from the beginning of the buffer to the
        // end of the buffer
        ring.buffer.resize(newSize);
        for (int i = currentSize; i < newSize; i++) 
        {
            ring.buffer[i] = ring.buffer[i - currentSize];
        }
        skr_atomic32_store_release(&ring.head, currentSize);
    }
    skr_atomic32_store_release(&ring.size, newSize);
    skr_release_rw_mutex(&rw_mutex);
}

template <typename T>
T resizable_ring_buffer<T>::add(T value) 
{
    skr_acquire_mutex_r(&rw_mutex);
    const auto old = ring.add(value);
    skr_release_rw_mutex(&rw_mutex);
    return old;
}

template <typename T>
T resizable_ring_buffer<T>::get(uint64_t index) 
{
    skr_acquire_mutex_r(&rw_mutex);
    auto result = ring.get(index);
    skr_release_rw_mutex(&rw_mutex);
    return result;
}

}