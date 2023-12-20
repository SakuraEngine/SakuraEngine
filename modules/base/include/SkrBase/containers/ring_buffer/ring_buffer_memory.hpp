#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/fwd_container.hpp"
#include "SkrBase/misc/debug.h"
#include "SkrBase/containers/misc/placeholder.hpp"
#include "SkrBase/containers/allocator/allocator.hpp"
#include "SkrBase/containers/ring_buffer/ring_buffer_heler.hpp"

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
            if (rhs.data())
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

// fixed ring buffer memory
namespace skr::container
{
template <typename T, typename TS, uint64_t kCount>
struct FixedRingBufferMemory {
    static_assert(kCount > 0, "FixedArrayMemory must have a capacity larger than 0");
    struct DummyParam {
    };
    using DataType           = T;
    using SizeType           = TS;
    using AllocatorCtorParam = DummyParam;

    // ctor & dtor
    inline FixedRingBufferMemory(AllocatorCtorParam param)
    {
    }
    inline ~FixedRingBufferMemory()
    {
        clear();
        free();
    }

    // copy & move
    inline FixedRingBufferMemory(const FixedRingBufferMemory& other, AllocatorCtorParam param)
    {
        if (other.size())
        {
            copy_ring_buffer(data(), other.data(), other.capacity(), other._front, other._back);
            _front = 0;
            _back  = other.size();
        }
    }
    inline FixedRingBufferMemory(FixedRingBufferMemory&& other) noexcept
    {
        if (other.size())
        {
            move_ring_buffer(data(), other.data(), other.capacity(), other._front, other._back);
            _front = 0;
            _back  = other.size();

            other._front = 0;
            other._back  = 0;
        }
    }

    // assign & move assign
    inline void operator=(const FixedRingBufferMemory& rhs) noexcept
    {
        if (this != &rhs)
        {
            // clean up self
            clear();

            // copy data
            if (rhs.size())
            {
                copy_ring_buffer(data(), rhs.data(), rhs.capacity(), rhs._front, rhs._back);
                _front = 0;
                _back  = rhs.size();
            }
        }
    }
    inline void operator=(FixedRingBufferMemory&& rhs) noexcept
    {
        if (this != &rhs)
        {
            // clean up self
            clear();

            // move data
            if (rhs.size())
            {
                move_ring_buffer(data(), rhs.data(), rhs.capacity(), rhs._front, rhs._back);
                _front = 0;
                _back  = rhs.size();

                rhs._front = 0;
                rhs._back  = 0;
            }
        }
    }

    // memory operations
    inline void realloc(SizeType new_capacity) noexcept
    {
        SKR_ASSERT(new_capacity <= kCount && "FixedRingBufferMemory can't alloc memory that larger than kCount");
    }
    inline void free() noexcept
    {
    }
    inline void grow_memory(SizeType grow_size) noexcept
    {
        SKR_ASSERT((size() + grow_size) <= kCount && "FixedRingBufferMemory can't alloc memory that larger than kCount");
    }
    inline void shrink() noexcept
    {
        // do noting
    }
    inline void clear() noexcept
    {
        if (size())
        {
            destruct_ring_buffer(data(), capacity(), _front, _back);
            _front = 0;
            _back  = 0;
        }
    }

    // getter
    inline T*       data() noexcept { return _placeholder.data_typed(); }
    inline const T* data() const noexcept { return _placeholder.data_typed(); }
    inline SizeType capacity() const noexcept { return kCount; }
    inline SizeType front() const noexcept { return _front; }
    inline SizeType back() const noexcept { return _back; }
    inline SizeType size() const noexcept { return _back - _front; }

    // setter
    inline void set_front(SizeType value) noexcept { _front = value; }
    inline void set_back(SizeType value) noexcept { _back = value; }

private:
    Placeholder<T, kCount> _placeholder;
    SizeType               _front = 0;
    SizeType               _back  = 0;
};
} // namespace skr::container

// inline ring buffer memory
namespace skr::container
{
template <typename T, typename TS, uint64_t kInlineCount, typename Allocator>
struct InlineRingBufferMemory : public Allocator {
    using DataType           = T;
    using SizeType           = TS;
    using AllocatorCtorParam = typename Allocator::CtorParam;

    // ctor & dtor
    inline InlineRingBufferMemory(AllocatorCtorParam param)
        : Allocator(std::move(param))
    {
    }
    inline ~InlineRingBufferMemory()
    {
        clear();
        free();
    }

    // copy & move
    inline InlineRingBufferMemory(const InlineRingBufferMemory& other, AllocatorCtorParam param)
        : Allocator(std::move(param))
    {
        if (other.size())
        {
            realloc(other.size());
            copy_ring_buffer(data(), other.data(), other._capacity, other._front, other._back);
            _front = 0;
            _back  = other.size();
        }
    }
    inline InlineRingBufferMemory(InlineRingBufferMemory&& other) noexcept
        : Allocator(std::move(other))
    {
        if (other._is_using_inline_memory())
        {
            // move inline data
            move_ring_buffer(data(), other.data(), other._capacity, other._front, other._back);
            _front = 0;
            _back  = other.size();

            // invalidate other
            other._front = 0;
            other._back  = 0;
        }
        else
        {
            // move heap data
            _heap_data = other._heap_data;
            _capacity  = other._capacity;
            _front     = other._front;
            _back      = other._back;

            // invalidate other
            other._heap_data = nullptr;
            other._capacity  = kInlineCount;
            other._front     = 0;
            other._back      = 0;
        }
    }

    // assign & move assign
    inline void operator=(const InlineRingBufferMemory& rhs) noexcept
    {
        if (this != &rhs)
        {
            // clean up self
            clear();

            // copy allocator
            Allocator::operator=(rhs);

            // copy data
            if (rhs.data())
            {
                // reserve memory
                if (capacity() < rhs.size())
                {
                    realloc(rhs.size());
                }

                // copy data
                copy_ring_buffer(data(), rhs.data(), rhs._capacity, rhs._front, rhs._back);
                _front = 0;
                _back  = rhs.size();
            }
        }
    }
    inline void operator=(InlineRingBufferMemory&& rhs) noexcept
    {
        if (this != &rhs)
        {
            if (rhs._is_using_inline_memory())
            {
                // clean up self
                clear();

                // move allocator
                Allocator::operator=(std::move(rhs));

                // move inline data
                move_ring_buffer(data(), rhs.data(), rhs._capacity, rhs._front, rhs._back);
                _front = 0;
                _back  = rhs.size();

                // invalidate other
                rhs._front = 0;
                rhs._back  = 0;
            }
            else
            {
                // clean up self
                clear();

                // free self
                free();

                // move allocator
                Allocator::operator=(std::move(rhs));

                // move data
                _heap_data = rhs._heap_data;
                _capacity  = rhs._capacity;
                _front     = rhs._front;
                _back      = rhs._back;

                // invalidate other
                rhs._heap_data = nullptr;
                rhs._capacity  = kInlineCount;
                rhs._front     = 0;
                rhs._back      = 0;
            }
        }
    }

    // memory operations
    inline void realloc(SizeType new_capacity) noexcept
    {
        SKR_ASSERT(new_capacity != _capacity);
        SKR_ASSERT(new_capacity > 0);
        SKR_ASSERT(size() <= new_capacity);

        if (new_capacity > kInlineCount)
        {
            if (_is_using_inline_memory()) // inline -> heap
            {
                // alloc new memory
                T* new_memory = Allocator::template alloc<T>(new_capacity);

                // move items
                if (size())
                {
                    move_ring_buffer(new_memory, _placeholder.data_typed(), _capacity, _front, _back);
                }

                // update data
                _heap_data = new_memory;
                _capacity  = new_capacity;
                _front     = 0;
                _back      = size();
            }
            else // heap -> heap
            {
                if constexpr (memory::MemoryTraits<T>::use_realloc && Allocator::support_realloc)
                {
                    // trim data to fit realloc logic
                    if (size())
                    {
                        trim_ring_buffer_for_new_capacity(_heap_data, _capacity, new_capacity, _front, _back);
                    }
                    _heap_data = Allocator::template realloc<T>(_heap_data, new_capacity);
                    _capacity  = new_capacity;
                }
                else
                {
                    // alloc new memory
                    T* new_memory = Allocator::template alloc<T>(new_capacity);

                    // copy data
                    SizeType cached_size = size();
                    if (cached_size)
                    {
                        copy_ring_buffer(new_memory, _heap_data, _capacity, _front, _back);
                    }

                    // release old memory
                    Allocator::template free<T>(_heap_data);

                    // update data
                    _heap_data = new_memory;
                    _capacity  = new_capacity;
                    _front     = 0;
                    _back      = cached_size;
                }
            }
        }
        else
        {
            if (_is_using_inline_memory()) // inline -> inline
            {
                // do noting
            }
            else // heap -> inline
            {
                T* cached_heap_data = _heap_data;

                // move items
                if (size())
                {
                    move_ring_buffer(_placeholder.data_typed(), cached_heap_data, _capacity, _front, _back);
                }

                // release old memory
                Allocator::template free<T>(cached_heap_data);

                // update data
                _capacity = kInlineCount;
                _front    = 0;
                _back     = size();
            }
        }
    }
    inline void free() noexcept
    {
        if (!_is_using_inline_memory())
        {
            Allocator::template free<T>(_heap_data);
            _heap_data = nullptr;
            _capacity  = kInlineCount;
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
            destruct_ring_buffer(data(), _capacity, _front, _back);
            _front = 0;
            _back  = 0;
        }
    }

    // getter
    inline T*       data() noexcept { return _is_using_inline_memory() ? _placeholder.data_typed() : _heap_data; }
    inline const T* data() const noexcept { return _is_using_inline_memory() ? _placeholder.data_typed() : _heap_data; }
    inline SizeType capacity() const noexcept { return _capacity; }
    inline SizeType front() const noexcept { return _front; }
    inline SizeType back() const noexcept { return _back; }
    inline SizeType size() const noexcept { return _back - _front; }

    // setter
    inline void set_front(SizeType value) noexcept { _front = value; }
    inline void set_back(SizeType value) noexcept { _back = value; }

private:
    // helper
    inline bool _is_using_inline_memory() const noexcept { return _capacity == kInlineCount; }

private:
    union
    {
        Placeholder<T, kInlineCount> _placeholder;
        T*                           _heap_data;
    };
    SizeType _capacity = kInlineCount;
    SizeType _front    = 0;
    SizeType _back     = 0;
};
} // namespace skr::container