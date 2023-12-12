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
    using SizeType           = TS;
    using AllocatorCtorParam = typename Allocator::CtorParam;

    // ctor & dtor
    inline ArrayMemory(AllocatorCtorParam param) noexcept
        : Allocator(std::move(param))
    {
    }
    inline ~ArrayMemory() noexcept
    {
        free();
    }

    // copy & move
    inline ArrayMemory(const ArrayMemory& other, AllocatorCtorParam param) noexcept
        : Allocator(std::move(param))
    {
        if (other._size > 0)
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
        SKR_ASSERT(this != &rhs && "before call operator=, you must check this != &rhs");
        SKR_ASSERT(_size == 0 && "before copy memory, you must clean up the memory first");

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
    inline void operator=(ArrayMemory&& rhs) noexcept
    {
        SKR_ASSERT(this != &rhs && "before call operator=, you must check this != &rhs");
        SKR_ASSERT(_data == nullptr && "before move memory, you must free the memory first");

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

    // getter
    inline T*       data() noexcept { return _data; }
    inline const T* data() const noexcept { return _data; }
    inline SizeType size() const noexcept { return _size; }
    inline SizeType capacity() const noexcept { return _capacity; }

    // setter
    inline void set_size(SizeType new_size) noexcept { _size = new_size; }

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
    using SizeType           = TS;
    using AllocatorCtorParam = DummyParam;

    // ctor & dtor
    inline FixedArrayMemory(AllocatorCtorParam) noexcept
    {
    }
    inline ~FixedArrayMemory() noexcept
    {
        free();
    }

    // copy & move
    inline FixedArrayMemory(const FixedArrayMemory& other, AllocatorCtorParam) noexcept
    {
        if (other._size > 0)
        {
            memory::copy(data(), other.data(), other._size);
            _size = other._size;
        }
    }
    inline FixedArrayMemory(FixedArrayMemory&& other) noexcept
    {
        if (other._size > 0)
        {
            memory::move(data(), other.data(), other._size);
            _size = other._size;

            other._size = 0;
        }
    }

    // assign & move assign
    inline void operator=(const FixedArrayMemory& rhs) noexcept
    {
        SKR_ASSERT(this != &rhs && "before call operator=, you must check this != &rhs");
        SKR_ASSERT(_size == 0 && "before copy memory, you must clean up the memory first");

        // copy data
        if (rhs._size > 0)
        {
            memory::copy(data(), rhs.data(), rhs._size);
            _size = rhs._size;
        }
    }
    inline void operator=(FixedArrayMemory&& rhs) noexcept
    {
        SKR_ASSERT(this != &rhs && "before call operator=, you must check this != &rhs");
        SKR_ASSERT(_size == 0 && "before move memory, you must clean up the memory first");

        // move data
        if (rhs._size > 0)
        {
            memory::move(data(), rhs.data(), rhs._size);
            _size = rhs._size;

            rhs._size = 0;
        }
    }

    // memory operations
    inline void realloc(SizeType new_capacity) noexcept
    {
        SKR_ASSERT(new_capacity <= kCount && "FixedArrayMemory can't alloc memory that larger than kCount");
    }
    inline void free() noexcept
    {
        if (_size)
        {
            memory::destruct(data(), _size);
            _size = 0;
        }
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

    // getter
    inline T*       data() noexcept { return reinterpret_cast<T*>(_placeholder.storage); }
    inline const T* data() const noexcept { return reinterpret_cast<const T*>(_placeholder.storage); }
    inline SizeType size() const noexcept { return _size; }
    inline SizeType capacity() const noexcept { return kCount; }

    // setter
    inline void set_size(SizeType new_size) noexcept { _size = new_size; }

private:
    Placeholder<T, kCount> _placeholder;
    SizeType               _size = 0;
};
} // namespace skr::container