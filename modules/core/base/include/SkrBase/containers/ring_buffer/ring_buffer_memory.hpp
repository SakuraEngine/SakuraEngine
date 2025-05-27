#pragma once
#include "SkrBase/config.h"
#include "SkrBase/misc/debug.h"
#include "SkrBase/containers/misc/placeholder.hpp"
#include "SkrBase/containers/misc/default_capicity_policy.hpp"
#include "SkrBase/containers/ring_buffer/ring_buffer_heler.hpp"

// ring buffer memory base
namespace skr::container
{
template <typename TSize>
struct RingBufferMemoryBase {
    using SizeType = TSize;

    // getter
    inline SizeType capacity() const noexcept { return _capacity; }
    inline SizeType front() const noexcept { return _front; }
    inline SizeType back() const noexcept { return _back; }
    inline SizeType size() const noexcept { return _back - _front; }

    // setter
    inline void set_front(SizeType value) noexcept { _front = value; }
    inline void set_back(SizeType value) noexcept { _back = value; }

protected:
    void*    _data     = nullptr;
    SizeType _capacity = 0;
    SizeType _front    = 0;
    SizeType _back     = 0;
};
} // namespace skr::container

// ring buffer memory
namespace skr::container
{
template <typename T, typename Base, typename Allocator>
struct RingBufferMemory : public Base, public Allocator {
    using DataType           = T;
    using SizeType           = typename Base::SizeType;
    using AllocatorCtorParam = typename Allocator::CtorParam;

    // ctor & dtor
    inline RingBufferMemory(AllocatorCtorParam param)
        : Base()
        , Allocator(std::move(param))
    {
    }
    inline ~RingBufferMemory()
    {
        clear();
        free();
    }

    // copy & move
    inline RingBufferMemory(const RingBufferMemory& rhs)
        : Base()
        , Allocator(rhs)
    {
        if (rhs.size())
        {
            realloc(rhs.size());
            copy_ring_buffer(data(), rhs.data(), rhs._capacity, rhs._front, rhs._back);
            Base::_front = 0;
            Base::_back  = rhs.size();
        }
    }
    inline RingBufferMemory(RingBufferMemory&& rhs) noexcept
        : Base(std::move(rhs))
        , Allocator(std::move(rhs))
    {
        rhs._reset();
    }

    // assign & move assign
    inline void operator=(const RingBufferMemory& rhs) noexcept
    {
        if (this != &rhs)
        {
            // copy allocator
            Allocator::operator=(rhs);

            // clean up self
            clear();

            // copy data
            if (rhs.data())
            {
                // reserve memory
                if (Base::_capacity < rhs.size())
                {
                    realloc(rhs.size());
                }

                // copy data
                copy_ring_buffer(data(), rhs.data(), rhs._capacity, rhs._front, rhs._back);
                Base::_front = 0;
                Base::_back  = rhs.size();
            }
        }
    }
    inline void operator=(RingBufferMemory&& rhs) noexcept
    {
        if (this != &rhs)
        {
            // move allocator
            Allocator::operator=(std::move(rhs));

            // clean up self
            clear();

            // free self
            free();

            // move data
            Base::_data     = rhs._data;
            Base::_capacity = rhs._capacity;
            Base::_front    = rhs._front;
            Base::_back     = rhs._back;

            // reset rhs
            rhs._reset();
        }
    }

    // memory operations
    inline void realloc(SizeType new_capacity) noexcept
    {
        SKR_ASSERT(new_capacity != Base::_capacity);
        SKR_ASSERT(new_capacity > 0);
        SKR_ASSERT(Base::size() <= new_capacity);
        SKR_ASSERT((Base::_capacity > 0 && Base::_data != nullptr) || (Base::_capacity == 0 && Base::_data == nullptr));

        // NOTE. 鉴于 ring buffer 元素分布的特殊性，realloc 未必会有性能优势，此处暂不实现

        // alloc new memory
        T* new_memory = Allocator::template alloc<T>(new_capacity);

        // copy data
        SizeType cached_size = Base::size();
        if (cached_size)
        {
            copy_ring_buffer(new_memory, data(), Base::_capacity, Base::_front, Base::_back);
        }

        // release old memory
        Allocator::template free<T>(data());

        // update data
        Base::_data     = new_memory;
        Base::_capacity = new_capacity;
        Base::_front    = 0;
        Base::_back     = cached_size;
    }
    inline void free() noexcept
    {
        if (data())
        {
            Allocator::template free<T>(data());
            Base::_data     = nullptr;
            Base::_capacity = 0;
        }
    }
    inline void grow_memory(SizeType grow_size) noexcept
    {
        SizeType new_size = Base::size() + grow_size;

        if (new_size > Base::_capacity)
        {
            SizeType new_capacity = default_get_grow<T>(new_size, Base::_capacity);
            SKR_ASSERT(new_capacity >= Base::_capacity);
            if (new_capacity >= Base::_capacity)
            {
                realloc(new_capacity);
            }
        }
    }
    inline void shrink() noexcept
    {
        SizeType new_capacity = default_get_shrink<T>(Base::size(), Base::_capacity);
        SKR_ASSERT(new_capacity >= Base::size());
        if (new_capacity < Base::_capacity)
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
        if (Base::size())
        {
            destruct_ring_buffer(data(), Base::_capacity, Base::_front, Base::_back);
            Base::_front = 0;
            Base::_back  = 0;
        }
    }

    // getter
    inline T*       data() noexcept { return reinterpret_cast<DataType*>(Base::_data); }
    inline const T* data() const noexcept { return reinterpret_cast<const DataType*>(Base::_data); }

private:
    inline void _reset()
    {
        Base::_data     = nullptr;
        Base::_capacity = 0;
        Base::_front    = 0;
        Base::_back     = 0;
    }
};
} // namespace skr::container

// fixed ring buffer memory
namespace skr::container
{
template <typename T, uint64_t kCount, typename Base>
struct FixedRingBufferMemory : public Base {
    static_assert(kCount > 0, "FixedRingBufferMemory must have a capacity larger than 0");
    struct DummyParam {
    };
    using DataType           = T;
    using SizeType           = typename Base::SizeType;
    using AllocatorCtorParam = DummyParam;

    // ctor & dtor
    inline FixedRingBufferMemory(AllocatorCtorParam param)
        : Base()
    {
        _init_setup();
    }
    inline ~FixedRingBufferMemory()
    {
        clear();
        free();
    }

    // copy & move
    inline FixedRingBufferMemory(const FixedRingBufferMemory& rhs)
    {
        _init_setup();

        if (rhs.size())
        {
            copy_ring_buffer(data(), rhs.data(), rhs.capacity(), rhs._front, rhs._back);
            Base::_front = 0;
            Base::_back  = rhs.size();
        }
    }
    inline FixedRingBufferMemory(FixedRingBufferMemory&& rhs) noexcept
    {
        _init_setup();

        if (rhs.size())
        {
            move_ring_buffer(data(), rhs.data(), rhs.capacity(), rhs._front, rhs._back);
            Base::_front = 0;
            Base::_back  = rhs.size();

            rhs._reset();
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
                Base::_front = 0;
                Base::_back  = rhs.size();
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
                Base::_front = 0;
                Base::_back  = rhs.size();
            }

            // reset rhs
            rhs._reset();
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
        SKR_ASSERT((Base::size() + grow_size) <= kCount && "FixedRingBufferMemory can't alloc memory that larger than kCount");
    }
    inline void shrink() noexcept
    {
        // do noting
    }
    inline void clear() noexcept
    {
        if (Base::size())
        {
            destruct_ring_buffer(data(), Base::_capacity, Base::_front, Base::_back);
            Base::_front = 0;
            Base::_back  = 0;
        }
    }

    // getter
    inline T*       data() noexcept { return reinterpret_cast<DataType*>(Base::_data); }
    inline const T* data() const noexcept { return reinterpret_cast<const DataType*>(Base::_data); }

private:
    inline void _init_setup()
    {
        Base::_data     = _placeholder.data();
        Base::_capacity = kCount;
    }

    inline void _reset()
    {
        Base::_front = 0;
        Base::_back  = 0;
    }

private:
    Placeholder<T, kCount> _placeholder;
};
} // namespace skr::container

// inline ring buffer memory
namespace skr::container
{
template <typename T, uint64_t kInlineCount, typename Base, typename Allocator>
struct InlineRingBufferMemory : public Base, public Allocator {
    using DataType           = T;
    using SizeType           = typename Base::SizeType;
    using AllocatorCtorParam = typename Allocator::CtorParam;

    // ctor & dtor
    inline InlineRingBufferMemory(AllocatorCtorParam param)
        : Base()
        , Allocator(std::move(param))
    {
        _reset();
    }
    inline ~InlineRingBufferMemory()
    {
        clear();
        free();
    }

    // copy & move
    inline InlineRingBufferMemory(const InlineRingBufferMemory& rhs)
        : Base()
        , Allocator(rhs)
    {
        _reset();

        if (rhs.size())
        {
            realloc(rhs.size());
            copy_ring_buffer(data(), rhs.data(), rhs._capacity, rhs._front, rhs._back);
            Base::_front = 0;
            Base::_back  = rhs.size();
        }
    }
    inline InlineRingBufferMemory(InlineRingBufferMemory&& rhs) noexcept
        : Base()
        , Allocator(std::move(rhs))
    {
        _reset();

        // move data
        if (rhs._is_using_inline_memory())
        {
            move_ring_buffer(data(), rhs.data(), rhs._capacity, rhs._front, rhs._back);
            Base::_front = 0;
            Base::_back  = rhs.size();
        }
        else
        {
            Base::_data     = rhs._data;
            Base::_capacity = rhs._capacity;
            Base::_front    = rhs._front;
            Base::_back     = rhs._back;
        }

        // reset rhs
        rhs._reset();
    }

    // assign & move assign
    inline void operator=(const InlineRingBufferMemory& rhs) noexcept
    {
        if (this != &rhs)
        {
            // copy allocator
            Allocator::operator=(rhs);

            // clean up self
            clear();

            // copy data
            if (rhs.data())
            {
                // reserve memory
                if (Base::_capacity < rhs.size())
                {
                    realloc(rhs.size());
                }

                // copy data
                copy_ring_buffer(data(), rhs.data(), rhs._capacity, rhs._front, rhs._back);
                Base::_front = 0;
                Base::_back  = rhs.size();
            }
        }
    }
    inline void operator=(InlineRingBufferMemory&& rhs) noexcept
    {
        if (this != &rhs)
        {
            // move allocator
            Allocator::operator=(std::move(rhs));

            // clean up self
            clear();
            free();

            // move data
            if (rhs._is_using_inline_memory())
            {
                // move inline data
                move_ring_buffer(data(), rhs.data(), rhs._capacity, rhs._front, rhs._back);
                Base::_front = 0;
                Base::_back  = rhs.size();
            }
            else
            {
                // move data
                Base::_data     = rhs._data;
                Base::_capacity = rhs._capacity;
                Base::_front    = rhs._front;
                Base::_back     = rhs._back;
            }

            // reset rhs
            rhs._reset();
        }
    }

    // memory operations
    inline void realloc(SizeType new_capacity) noexcept
    {
        SKR_ASSERT(new_capacity != Base::_capacity || new_capacity <= kInlineCount);
        SKR_ASSERT(new_capacity > 0);
        SKR_ASSERT(Base::size() <= new_capacity);
        new_capacity = new_capacity < kInlineCount ? kInlineCount : new_capacity;

        // update data
        if (new_capacity > kInlineCount)
        {
            if (_is_using_inline_memory()) // inline -> heap
            {
                // alloc new memory
                T* new_memory = Allocator::template alloc<T>(new_capacity);

                // move items
                SizeType cached_size = Base::size();
                if (cached_size)
                {
                    move_ring_buffer(new_memory, _placeholder.data_typed(), Base::_capacity, Base::_front, Base::_back);
                }

                // update data
                Base::_data  = new_memory;
                Base::_front = 0;
                Base::_back  = cached_size;
            }
            else // heap -> heap
            {

                // NOTE. 鉴于 ring buffer 元素分布的特殊性，realloc 未必会有性能优势，此处暂不实现

                // alloc new memory
                T* new_memory = Allocator::template alloc<T>(new_capacity);

                // copy data
                SizeType cached_size = Base::size();
                if (cached_size)
                {
                    copy_ring_buffer(new_memory, data(), Base::_capacity, Base::_front, Base::_back);
                }

                // release old memory
                Allocator::template free<T>(data());

                // update data
                Base::_data  = new_memory;
                Base::_front = 0;
                Base::_back  = cached_size;
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
                T* cached_heap_data = data();

                // move items
                SizeType cached_size = Base::size();
                if (cached_size)
                {
                    move_ring_buffer(_placeholder.data_typed(), cached_heap_data, Base::_capacity, Base::_front, Base::_back);
                }

                // release old memory
                Allocator::template free<T>(cached_heap_data);

                // update data
                Base::_front = 0;
                Base::_back  = cached_size;
            }
        }

        // update capacity
        Base::_capacity = new_capacity;
    }
    inline void free() noexcept
    {
        if (!_is_using_inline_memory())
        {
            Allocator::template free<T>(data());
            _reset();
        }
    }
    inline void grow_memory(SizeType grow_size) noexcept
    {
        SizeType new_size = Base::size() + grow_size;

        if (new_size > Base::_capacity)
        {
            SizeType new_capacity = default_get_grow<T>(new_size, Base::_capacity);
            SKR_ASSERT(new_capacity >= Base::_capacity);
            if (new_capacity >= Base::_capacity)
            {
                realloc(new_capacity);
            }
        }
    }
    inline void shrink() noexcept
    {
        SizeType new_capacity = default_get_shrink<T>(Base::size(), Base::_capacity);
        SKR_ASSERT(new_capacity >= Base::size());
        if (new_capacity < Base::_capacity)
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
        if (Base::size())
        {
            destruct_ring_buffer(data(), Base::_capacity, Base::_front, Base::_back);
            Base::_front = 0;
            Base::_back  = 0;
        }
    }

    // getter
    inline T*       data() noexcept { return reinterpret_cast<DataType*>(Base::_data); }
    inline const T* data() const noexcept { return reinterpret_cast<const DataType*>(Base::_data); }

private:
    // helper
    inline bool _is_using_inline_memory() const noexcept { return Base::_data == _placeholder.data_typed(); }

    inline void _reset()
    {
        Base::_data     = _placeholder.data_typed();
        Base::_capacity = kInlineCount;
        Base::_front    = 0;
        Base::_back     = 0;
    }

private:
    Placeholder<T, kInlineCount> _placeholder;
};
} // namespace skr::container