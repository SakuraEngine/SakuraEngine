#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/fwd_container.hpp"
#include "SkrBase/misc/debug.h"
#include "SkrBase/containers/misc/placeholder.hpp"
#include "SkrBase/containers/allocator/allocator.hpp"
#include "SkrBase/memory/memory_ops.hpp"

// util array memory
namespace skr::container
{
template <typename T, typename TS, typename Allocator>
struct ArrayMemory : public Allocator {
    using DataType           = T;
    using SizeType           = TS;
    using AllocatorCtorParam = typename Allocator::CtorParam;

    // ctor & dtor
    inline ArrayMemory(AllocatorCtorParam param) noexcept
        : Allocator(std::move(param))
    {
    }
    inline ~ArrayMemory() noexcept
    {
        clear();
        free();
    }

    // copy & move
    inline ArrayMemory(const ArrayMemory& other, AllocatorCtorParam param) noexcept
        : Allocator(std::move(param))
    {
        if (other._size)
        {
            realloc(other._size);
            memory::copy(_data, other._data, other._size);
            _size = other._size;
        }
    }
    inline ArrayMemory(ArrayMemory&& other) noexcept
        : Allocator(std::move(other))
        , _data(other._data)
        , _size(other._size)
        , _capacity(other._capacity)
    {
        other._data     = nullptr;
        other._size     = 0;
        other._capacity = 0;
    }

    // assign & move assign
    inline void operator=(const ArrayMemory& rhs) noexcept
    {
        if (this != &rhs)
        {
            // clean up self
            clear();

            // copy allocator
            Allocator::operator=(rhs);

            // copy data
            if (rhs._size > 0)
            {
                // reserve memory
                if (_capacity < rhs._size)
                {
                    realloc(rhs._size);
                }

                // copy data
                memory::copy(_data, rhs._data, rhs._size);
                _size = rhs._size;
            }
        }
    }
    inline void operator=(ArrayMemory&& rhs) noexcept
    {
        if (this != &rhs)
        {
            // clean up self
            clear();

            // free
            free();

            // move allocator
            Allocator::operator=(std::move(rhs));

            // move data
            _data         = rhs._data;
            _size         = rhs._size;
            _capacity     = rhs._capacity;
            rhs._data     = nullptr;
            rhs._size     = 0;
            rhs._capacity = 0;
        }
    }

    // memory operations
    inline void realloc(SizeType new_capacity) noexcept
    {
        SKR_ASSERT(new_capacity != _capacity);
        SKR_ASSERT(new_capacity > 0);
        SKR_ASSERT(_size <= new_capacity);
        SKR_ASSERT((_capacity > 0 && _data != nullptr) || (_capacity == 0 && _data == nullptr));

        if constexpr (memory::MemoryTraits<T>::use_realloc && Allocator::support_realloc)
        {
            _data     = Allocator::template realloc<T>(_data, new_capacity);
            _capacity = new_capacity;
        }
        else
        {
            // alloc new memory
            T* new_memory = Allocator::template alloc<T>(new_capacity);

            // move items
            if (_size)
            {
                memory::move(new_memory, _data, _size);
            }

            // release old memory
            Allocator::template free<T>(_data);

            // update data
            _data     = new_memory;
            _capacity = new_capacity;
        }
    }
    inline void free() noexcept
    {
        if (_data)
        {
            Allocator::template free<T>(_data);
            _data     = nullptr;
            _size     = 0;
            _capacity = 0;
        }
    }
    inline SizeType grow(SizeType grow_size) noexcept
    {
        SizeType old_size = _size;
        SizeType new_size = old_size + grow_size;

        if (new_size > _capacity)
        {
            SizeType new_capacity = default_get_grow<T>(new_size, _capacity);
            SKR_ASSERT(new_capacity >= _capacity);
            if (new_capacity >= _capacity)
            {
                realloc(new_capacity);
            }
        }

        _size = new_size;
        return old_size;
    }
    inline void shrink() noexcept
    {
        SizeType new_capacity = default_get_shrink<T>(_size, _capacity);
        SKR_ASSERT(new_capacity >= _size);
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
        if (_size)
        {
            memory::destruct(_data, _size);
            _size = 0;
        }
    }

    // getter
    inline T*       data() noexcept { return _data; }
    inline const T* data() const noexcept { return _data; }
    inline SizeType size() const noexcept { return _size; }
    inline SizeType capacity() const noexcept { return _capacity; }

    // setter
    inline void set_size(SizeType value) noexcept { _size = value; }

private:
    T*       _data     = nullptr;
    SizeType _size     = 0;
    SizeType _capacity = 0;
};
} // namespace skr::container

// fixed array memory
namespace skr::container
{
template <typename T, typename TS, uint64_t kCount>
struct FixedArrayMemory {
    static_assert(kCount > 0, "FixedArrayMemory must have a capacity larger than 0");
    struct DummyParam {
    };
    using DataType           = T;
    using SizeType           = TS;
    using AllocatorCtorParam = DummyParam;

    // ctor & dtor
    inline FixedArrayMemory(AllocatorCtorParam) noexcept
    {
    }
    inline ~FixedArrayMemory() noexcept
    {
        clear();
        free();
    }

    // copy & move
    inline FixedArrayMemory(const FixedArrayMemory& other, AllocatorCtorParam) noexcept
    {
        if (other._size)
        {
            memory::copy(data(), other.data(), other._size);
            _size = other._size;
        }
    }
    inline FixedArrayMemory(FixedArrayMemory&& other) noexcept
    {
        if (other._size)
        {
            memory::move(data(), other.data(), other._size);
            _size = other._size;

            other._size = 0;
        }
    }

    // assign & move assign
    inline void operator=(const FixedArrayMemory& rhs) noexcept
    {
        if (this != &rhs)
        {
            // clean up self
            clear();

            // copy data
            if (rhs._size > 0)
            {
                memory::copy(data(), rhs.data(), rhs._size);
                _size = rhs._size;
            }
        }
    }
    inline void operator=(FixedArrayMemory&& rhs) noexcept
    {
        if (this != &rhs)
        {
            // clean up self
            clear();

            // move data
            if (rhs._size > 0)
            {
                memory::move(data(), rhs.data(), rhs._size);
                _size = rhs._size;

                rhs._size = 0;
            }
        }
    }

    // memory operations
    inline void realloc(SizeType new_capacity) noexcept
    {
        SKR_ASSERT(new_capacity <= kCount && "FixedArrayMemory can't alloc memory that larger than kCount");
    }
    inline void free() noexcept
    {
    }
    inline SizeType grow(SizeType grow_size) noexcept
    {
        SKR_ASSERT((_size + grow_size) <= kCount && "FixedArrayMemory can't alloc memory that larger than kCount");

        SizeType old_size = _size;
        _size += grow_size;
        return old_size;
    }
    inline void shrink() noexcept
    {
        // do noting
    }
    inline void clear() noexcept
    {
        if (_size)
        {
            memory::destruct(data(), _size);
            _size = 0;
        }
    }

    // getter
    inline T*       data() noexcept { return _placeholder.data_typed(); }
    inline const T* data() const noexcept { return _placeholder.data_typed(); }
    inline SizeType size() const noexcept { return _size; }
    inline SizeType capacity() const noexcept { return kCount; }

    // setter
    inline void set_size(SizeType value) noexcept { _size = value; }

private:
    Placeholder<T, kCount> _placeholder;
    SizeType               _size = 0;
};
} // namespace skr::container

// inline array memory
namespace skr::container
{
template <typename T, typename TS, uint64_t kInlineCount, typename Allocator>
struct InlineArrayMemory : public Allocator {
    using DataType           = T;
    using SizeType           = TS;
    using AllocatorCtorParam = typename Allocator::CtorParam;

    // ctor & dtor
    inline InlineArrayMemory(AllocatorCtorParam param) noexcept
        : Allocator(std::move(param))
    {
    }
    inline ~InlineArrayMemory() noexcept
    {
        clear();
        free();
    }

    // copy & move
    inline InlineArrayMemory(const InlineArrayMemory& other, AllocatorCtorParam param) noexcept
        : Allocator(std::move(param))
    {
        if (other._size)
        {
            realloc(other._size);
            memory::copy(data(), other.data(), other._size);
            _size = other._size;
        }
    }
    inline InlineArrayMemory(InlineArrayMemory&& other) noexcept
        : Allocator(std::move(other))
    {
        if (other._is_using_inline_memory())
        {
            // move inline data
            memory::move(_placeholder.data_typed(), other._placeholder.data_typed(), other._size);
            _size = other._size;

            // invalidate other
            other._size = 0;
        }
        else
        {
            // move heap data
            _heap_data = other._heap_data;
            _size      = other._size;
            _capacity  = other._capacity;

            // invalidate other
            other._heap_data = nullptr;
            other._size      = 0;
            other._capacity  = kInlineCount;
        }
    }

    // assign & move assign
    inline void operator=(const InlineArrayMemory& rhs) noexcept
    {
        if (this != &rhs)
        {
            // clean up self
            clear();

            // copy allocator
            Allocator::operator=(rhs);

            // copy data
            if (rhs._size > 0)
            {
                // reserve memory
                if (_capacity < rhs._size)
                {
                    realloc(rhs._size);
                }

                // copy data
                memory::copy(data(), rhs.data(), rhs._size);
                _size = rhs._size;
            }
        }
    }
    inline void operator=(InlineArrayMemory&& rhs) noexcept
    {
        if (this != &rhs)
        {
            // move data
            if (rhs._is_using_inline_memory())
            {
                // clean up self
                clear();

                // move allocator
                Allocator::operator=(std::move(rhs));

                // move inline data
                memory::move(data(), rhs.data(), rhs._size);
                _size = rhs._size;

                // invalidate rhs
                rhs._size = 0;
            }
            else
            {
                // clean up self
                clear();

                // free
                free();

                // move allocator
                Allocator::operator=(std::move(rhs));

                // move data
                _heap_data = rhs._heap_data;
                _size      = rhs._size;
                _capacity  = rhs._capacity;

                // invalidate rhs
                rhs._heap_data = rhs._heap_data;
                rhs._size      = 0;
                rhs._capacity  = kInlineCount;
            }
        }
    }

    // memory operations
    inline void realloc(SizeType new_capacity) noexcept
    {
        SKR_ASSERT(new_capacity != _capacity);
        SKR_ASSERT(new_capacity > 0);
        SKR_ASSERT(_size <= new_capacity);

        if (new_capacity > kInlineCount)
        {
            if (_is_using_inline_memory()) // inline -> heap
            {
                // alloc new memory
                T* new_memory = Allocator::template alloc<T>(new_capacity);

                // move items
                if (_size)
                {
                    memory::move(new_memory, _placeholder.data_typed(), _size);
                }

                // update data
                _heap_data = new_memory;
                _capacity  = new_capacity;
            }
            else // heap -> heap
            {
                if constexpr (memory::MemoryTraits<T>::use_realloc && Allocator::support_realloc)
                {
                    _heap_data = Allocator::template realloc<T>(_heap_data, new_capacity);
                    _capacity  = new_capacity;
                }
                else
                {
                    // alloc new memory
                    T* new_memory = Allocator::template alloc<T>(new_capacity);

                    // move items
                    if (_size)
                    {
                        memory::move(new_memory, _heap_data, _size);
                    }

                    // release old memory
                    Allocator::template free<T>(_heap_data);

                    // update data
                    _heap_data = new_memory;
                    _capacity  = new_capacity;
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
                if (_size)
                {
                    memory::move(_placeholder.data_typed(), cached_heap_data, _size);
                }

                // release old memory
                Allocator::template free<T>(cached_heap_data);

                // update data
                _capacity = kInlineCount;
            }
        }
    }
    inline void free() noexcept
    {
        if (!_is_using_inline_memory())
        {
            Allocator::template free<T>(_heap_data);
            _heap_data = nullptr;
            _size      = 0;
            _capacity  = kInlineCount;
        }
    }
    inline SizeType grow(SizeType grow_size) noexcept
    {
        SizeType old_size = _size;
        SizeType new_size = old_size + grow_size;

        if (new_size > _capacity)
        {
            SizeType new_capacity = default_get_grow<T>(new_size, _capacity);
            SKR_ASSERT(new_capacity >= _capacity);
            if (new_capacity >= _capacity)
            {
                realloc(new_capacity);
            }
        }

        _size = new_size;
        return old_size;
    }
    inline void shrink() noexcept
    {
        SizeType new_capacity = default_get_shrink<T>(_size, _capacity);
        SKR_ASSERT(new_capacity >= _size);
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
        if (_size)
        {
            memory::destruct(data(), _size);
            _size = 0;
        }
    }

    // getter
    inline T*       data() noexcept { return _is_using_inline_memory() ? _placeholder.data_typed() : _heap_data; }
    inline const T* data() const noexcept { return _is_using_inline_memory() ? _placeholder.data_typed() : _heap_data; }
    inline SizeType size() const noexcept { return _size; }
    inline SizeType capacity() const noexcept { return _capacity; }

    // setter
    inline void set_size(SizeType value) noexcept { _size = value; }

private:
    // helper
    inline bool _is_using_inline_memory() const noexcept { return _capacity == kInlineCount; }

private:
    union
    {
        Placeholder<T, kInlineCount> _placeholder;
        T*                           _heap_data;
    };
    SizeType _size     = 0;
    SizeType _capacity = kInlineCount;
};
} // namespace skr::container