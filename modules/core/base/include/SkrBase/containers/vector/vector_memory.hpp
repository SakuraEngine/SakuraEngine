#pragma once
#include "SkrBase/config.h"
#include "SkrBase/misc/debug.h"
#include "SkrBase/containers/misc/placeholder.hpp"
#include "SkrBase/containers/misc/default_capicity_policy.hpp"
#include "SkrBase/memory/memory_ops.hpp"

// util vector memory
namespace skr::container
{
template <typename T, typename TS, typename Allocator>
struct VectorMemory : public Allocator {
    using DataType           = T;
    using SizeType           = TS;
    using AllocatorCtorParam = typename Allocator::CtorParam;

    // ctor & dtor
    inline VectorMemory(AllocatorCtorParam param) noexcept
        : Allocator(std::move(param))
    {
    }
    inline ~VectorMemory() noexcept
    {
        clear();
        free();
    }

    // copy & move
    inline VectorMemory(const VectorMemory& rhs) noexcept
        : Allocator(rhs)
    {
        if (rhs._size)
        {
            realloc(rhs._size);
            memory::copy(_data, rhs._data, rhs._size);
            _size = rhs._size;
        }
    }
    inline VectorMemory(VectorMemory&& rhs) noexcept
        : Allocator(std::move(rhs))
        , _data(rhs._data)
        , _size(rhs._size)
        , _capacity(rhs._capacity)
    {
        rhs._reset();
    }

    // assign & move assign
    inline void operator=(const VectorMemory& rhs) noexcept
    {
        if (this != &rhs)
        {
            // copy allocator
            Allocator::operator=(rhs);

            // clean up self
            clear();

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
    inline void operator=(VectorMemory&& rhs) noexcept
    {
        if (this != &rhs)
        {
            // move allocator
            Allocator::operator=(std::move(rhs));

            // clean up self
            clear();
            free();

            // move data
            _data     = rhs._data;
            _size     = rhs._size;
            _capacity = rhs._capacity;

            // clean up rhs
            rhs._reset();
        }
    }

    // memory operations
    inline void realloc(SizeType new_capacity) noexcept
    {
        SKR_ASSERT(new_capacity != _capacity);
        SKR_ASSERT(new_capacity > 0);
        SKR_ASSERT(_size <= new_capacity);
        SKR_ASSERT((_capacity > 0 && _data != nullptr) || (_capacity == 0 && _data == nullptr));

        // update memory
        if constexpr (memory::MemoryTraits<T>::use_realloc && Allocator::support_realloc)
        {
            _data = Allocator::template realloc<T>(_data, new_capacity);
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
            _data = new_memory;
        }

        // update capacity
        _capacity = new_capacity;
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
    // helper functions
    inline void _reset() noexcept
    {
        _data     = nullptr;
        _size     = 0;
        _capacity = 0;
    }

private:
    T*       _data     = nullptr;
    SizeType _size     = 0;
    SizeType _capacity = 0;
};
} // namespace skr::container

// fixed vector memory
namespace skr::container
{
template <typename T, typename TS, uint64_t kCount>
struct FixedVectorMemory {
    static_assert(kCount > 0, "FixedVectorMemory must have a capacity larger than 0");
    struct DummyParam {
    };
    using DataType           = T;
    using SizeType           = TS;
    using AllocatorCtorParam = DummyParam;

    // ctor & dtor
    inline FixedVectorMemory(AllocatorCtorParam) noexcept
    {
    }
    inline ~FixedVectorMemory() noexcept
    {
        clear();
        free();
    }

    // copy & move
    inline FixedVectorMemory(const FixedVectorMemory& other) noexcept
    {
        if (other._size)
        {
            memory::copy(data(), other.data(), other._size);
            _size = other._size;
        }
    }
    inline FixedVectorMemory(FixedVectorMemory&& other) noexcept
    {
        if (other._size)
        {
            memory::move(data(), other.data(), other._size);
            _size = other._size;

            other._reset();
        }
    }

    // assign & move assign
    inline void operator=(const FixedVectorMemory& rhs) noexcept
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
    inline void operator=(FixedVectorMemory&& rhs) noexcept
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

                rhs._reset();
            }
        }
    }

    // memory operations
    inline void realloc(SizeType new_capacity) noexcept
    {
        SKR_ASSERT(new_capacity <= kCount && "FixedVectorMemory can't alloc memory that larger than kCount");
    }
    inline void free() noexcept
    {
    }
    inline SizeType grow(SizeType grow_size) noexcept
    {
        SKR_ASSERT((_size + grow_size) <= kCount && "FixedVectorMemory can't alloc memory that larger than kCount");

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
    inline void _reset() noexcept
    {
        _size = 0;
    }

private:
    Placeholder<T, kCount> _placeholder;
    SizeType               _size = 0;
};
} // namespace skr::container

// inline vector memory
namespace skr::container
{
template <typename T, typename TS, uint64_t kInlineCount, typename Allocator>
struct InlineVectorMemory : public Allocator {
    using DataType           = T;
    using SizeType           = TS;
    using AllocatorCtorParam = typename Allocator::CtorParam;

    // ctor & dtor
    inline InlineVectorMemory(AllocatorCtorParam param) noexcept
        : Allocator(std::move(param))
    {
    }
    inline ~InlineVectorMemory() noexcept
    {
        clear();
        free();
    }

    // copy & move
    inline InlineVectorMemory(const InlineVectorMemory& rhs) noexcept
        : Allocator(rhs)
    {
        if (rhs._size)
        {
            realloc(rhs._size);
            memory::copy(data(), rhs.data(), rhs._size);
            _size = rhs._size;
        }
    }
    inline InlineVectorMemory(InlineVectorMemory&& rhs) noexcept
        : Allocator(std::move(rhs))
    {
        // move data
        if (rhs._is_using_inline_memory())
        {
            memory::move(_placeholder.data_typed(), rhs._placeholder.data_typed(), rhs._size);
            _size = rhs._size;
        }
        else
        {
            _heap_data = rhs._heap_data;
            _size      = rhs._size;
            _capacity  = rhs._capacity;
        }

        // reset rhs
        rhs._reset();
    }

    // assign & move assign
    inline void operator=(const InlineVectorMemory& rhs) noexcept
    {
        if (this != &rhs)
        {
            // copy allocator
            Allocator::operator=(rhs);

            // clean up self
            clear();

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
    inline void operator=(InlineVectorMemory&& rhs) noexcept
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
                memory::move(data(), rhs.data(), rhs._size);
                _size = rhs._size;
            }
            else
            {
                _heap_data = rhs._heap_data;
                _size      = rhs._size;
                _capacity  = rhs._capacity;
            }

            // reset rhs
            rhs._reset();
        }
    }

    // memory operations
    inline void realloc(SizeType new_capacity) noexcept
    {
        SKR_ASSERT(new_capacity != _capacity);
        SKR_ASSERT(new_capacity > 0);
        SKR_ASSERT(_size <= new_capacity);
        new_capacity = new_capacity < kInlineCount ? kInlineCount : new_capacity;

        // update data
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
            }
        }

        // update capacity
        _capacity = new_capacity;
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
    inline void _reset()
    {
        _size     = 0;
        _capacity = kInlineCount;
    }

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