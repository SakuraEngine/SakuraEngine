#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/fwd_container.hpp"
#include "SkrBase/misc/debug.h"
#include "SkrBase/containers/misc/placeholder.hpp"
#include "SkrBase/containers/allocator/allocator.hpp"
#include "SkrBase/memory/memory_ops.hpp"

// helper
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
inline void move_ring_buffer(T* dst, const T* src, TS src_capacity, TS src_front, TS src_back) noexcept
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

// ring buffer memory
namespace skr::container
{
template <typename T, typename TS, typename Allocator>
struct RingBufferMemory : public Allocator {
    using DataType           = T;
    using SizeType           = TS;
    using AllocatorCtorParam = typename Allocator::CtorParam;

    // ctor & dtor
    inline RingBufferMemory(AllocatorCtorParam param)
        : Allocator(std::move(param))
    {
    }
    inline ~RingBufferMemory()
    {
        clear();
        free();
    }

    // copy & move
    inline RingBufferMemory(const RingBufferMemory& other, AllocatorCtorParam param)
        : Allocator(std::move(param))
    {
        if (other.size())
        {
            realloc(other.size());
            copy_ring_buffer(_data, other._data, other._capacity, other._front, other._back);
            _front = 0;
            _back  = other.size();
        }
    }
    inline RingBufferMemory(RingBufferMemory&& other) noexcept
        : Allocator(std::move(other))
        , _data(other._data)
        , _capacity(other._capacity)
        , _front(other._front)
        , _back(other._back)
    {
        other._data     = nullptr;
        other._capacity = 0;
        other._front    = 0;
        other._back     = 0;
    }

    // assign & move assign
    inline void operator=(const RingBufferMemory& rhs) noexcept
    {
        if (this != &rhs)
        {
            // clean up self
            clear();

            // copy allocator
            Allocator::operator=(rhs);

            // copy data
            if (rhs.data() > 0)
            {
                // reserve memory
                if (capacity() < rhs.size())
                {
                    realloc(rhs.size());
                }

                // copy data
                copy_ring_buffer(_data, rhs._data, rhs._capacity, rhs._front, rhs._back);
                _front = 0;
                _back  = rhs.size();
            }
        }
    }
    inline void operator=(RingBufferMemory&& rhs) noexcept
    {
        if (this != &rhs)
        {
            // clean up self
            clear();

            // free self
            free();

            // move allocator
            Allocator::operator=(std::move(rhs));

            // move data
            _data         = rhs._data;
            _capacity     = rhs._capacity;
            _front        = rhs._front;
            _back         = rhs._back;
            rhs._data     = nullptr;
            rhs._capacity = 0;
            rhs._front    = 0;
            rhs._back     = 0;
        }
    }

    // memory operations
    inline void realloc(SizeType new_capacity) noexcept
    {
        SKR_ASSERT(new_capacity != _capacity);
        SKR_ASSERT(new_capacity > 0);
        SKR_ASSERT(size() <= new_capacity);
        SKR_ASSERT((_capacity > 0 && _data != nullptr) || (_capacity == 0 && _data == nullptr));

        if constexpr (memory::MemoryTraits<T>::use_realloc && Allocator::support_realloc)
        {
            // trim data to fit realloc logic
            if (size())
            {
                trim_ring_buffer_for_new_capacity(_data, _capacity, new_capacity, _front, _back);
            }
            _data     = Allocator::template realloc<T>(_data, new_capacity);
            _capacity = new_capacity;
        }
        else
        {
            // alloc new memory
            T* new_memory = Allocator::template alloc<T>(new_capacity);

            // copy data
            SizeType cached_size = size();
            if (cached_size)
            {
                copy_ring_buffer(new_memory, _data, _capacity, _front, _back);
            }

            // release old memory
            Allocator::template free<T>(_data);

            // update data
            _data     = new_memory;
            _capacity = new_capacity;
            _front    = 0;
            _back     = cached_size;
        }
    }
    inline void free() noexcept
    {
        if (_data)
        {
            Allocator::template free<T>(_data);
            _data     = nullptr;
            _capacity = 0;
        }
    }
    inline void grow_memory(SizeType grow_size) noexcept
    {
        SizeType new_size = size() + grow_size;

        if (new_size > _capacity)
        {
            SizeType new_capacity = default_get_grow<T>(new_size, _capacity);
            SKR_ASSERT(new_capacity >= _capacity);
            if (new_capacity >= _capacity)
            {
                realloc(new_capacity);
            }
        }
    }
    inline void shrink() noexcept
    {
        SizeType new_capacity = default_get_shrink<T>(size(), _capacity);
        SKR_ASSERT(new_capacity >= size());
        if (new_capacity < _capacity)
        {
            if (new_capacity)
            {
                realloc(new_capacity);
            }
            else
            {
                free();
            }
        }
    }
    inline void clear() noexcept
    {
        if (size())
        {
            destruct_ring_buffer(_data, _capacity, _front, _back);
            _front = 0;
            _back  = 0;
        }
    }

    // getter
    inline T*       data() noexcept { return _data; }
    inline const T* data() const noexcept { return _data; }
    inline SizeType capacity() const noexcept { return _capacity; }
    inline SizeType front() const noexcept { return _front; }
    inline SizeType back() const noexcept { return _back; }
    inline SizeType size() const noexcept { return _back - _front; }

    // setter
    inline void set_front(SizeType value) noexcept { _front = value; }
    inline void set_back(SizeType value) noexcept { _back = value; }

private:
    T*       _data     = nullptr;
    SizeType _capacity = 0;
    SizeType _front    = 0;
    SizeType _back     = 0;
};
} // namespace skr::container