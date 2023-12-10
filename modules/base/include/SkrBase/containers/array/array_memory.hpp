#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/fwd_container.hpp"
#include "SkrBase/misc/debug.h"
#include "SkrBase/containers/misc/placeholder.hpp"
#include "SkrBase/containers/allocator/allocator.hpp"
#include "SkrBase/memory/memory_ops.hpp"

namespace skr::container
{
template <typename T, typename TS, typename Allocator>
struct ArrayMemory : public Allocator {
    using SizeType           = TS;
    using AllocatorCtorParam = typename Allocator::CtorParam;

    // ctor & dtor
    ArrayMemory(AllocatorCtorParam param) noexcept;
    ~ArrayMemory() noexcept;

    // copy & move
    ArrayMemory(const ArrayMemory& other, AllocatorCtorParam param) noexcept;
    ArrayMemory(ArrayMemory&& other) noexcept;

    // assign & move assign
    ArrayMemory& operator=(const ArrayMemory& rhs) noexcept;
    ArrayMemory& operator=(ArrayMemory&& rhs) noexcept;

    // memory operations
    void realloc(SizeType new_capacity) noexcept;
    void free() noexcept;
    void grow(SizeType new_size) noexcept;
    void shrink() noexcept;

    // getter
    T*       data() noexcept;
    const T* data() const noexcept;
    SizeType size() const noexcept;
    SizeType capacity() const noexcept;

    // setter
    void set_size(SizeType new_size) noexcept;

private:
    T*       _data     = nullptr;
    SizeType _size     = 0;
    SizeType _capacity = 0;
};

template <typename T, typename TS, uint64_t kCount>
struct FixedArrayMemory {
    static_assert(kCount > 0, "FixedArrayMemory must have a capacity larger than 0");
    struct DummyParam {
    };
    using SizeType           = TS;
    using AllocatorCtorParam = DummyParam;

    // ctor & dtor
    FixedArrayMemory(AllocatorCtorParam) noexcept;
    ~FixedArrayMemory() noexcept;

    // copy & move
    FixedArrayMemory(const FixedArrayMemory& other, AllocatorCtorParam) noexcept;
    FixedArrayMemory(FixedArrayMemory&& other) noexcept;

    // assign & move assign
    FixedArrayMemory& operator=(const FixedArrayMemory& rhs) noexcept;
    FixedArrayMemory& operator=(FixedArrayMemory&& rhs) noexcept;

    // memory operations
    void realloc(SizeType new_capacity) noexcept;
    void free() noexcept;
    void grow(SizeType new_size) noexcept;
    void shrink() noexcept;

    // getter
    T*       data() noexcept;
    const T* data() const noexcept;
    SizeType size() const noexcept;
    SizeType capacity() const noexcept;

    // setter
    void set_size(SizeType new_size) noexcept;

private:
    Placeholder<T, kCount> _placeholder;
    SizeType               _size = 0;
};
} // namespace skr::container

// util array memory
namespace skr::container
{
// ctor & dtor
template <typename T, typename TS, typename Allocator>
inline ArrayMemory<T, TS, Allocator>::ArrayMemory(AllocatorCtorParam param) noexcept
    : Allocator(std::move(param))
{
}
template <typename T, typename TS, typename Allocator>
inline ArrayMemory<T, TS, Allocator>::~ArrayMemory() noexcept
{
    free();
}

// copy & move
template <typename T, typename TS, typename Allocator>
inline ArrayMemory<T, TS, Allocator>::ArrayMemory(const ArrayMemory& other, AllocatorCtorParam param) noexcept
    : Allocator(std::move(param))
{
    if (other._size > 0)
    {
        realloc(other._size);
        memory::copy(_data, other._data, other._size);
        _size = other._size;
    }
}
template <typename T, typename TS, typename Allocator>
inline ArrayMemory<T, TS, Allocator>::ArrayMemory(ArrayMemory&& other) noexcept
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
template <typename T, typename TS, typename Allocator>
inline ArrayMemory<T, TS, Allocator>& ArrayMemory<T, TS, Allocator>::operator=(const ArrayMemory& rhs) noexcept
{
    if (this != &rhs)
    {
        // clear
        free();

        // copy data
        if (rhs._size > 0)
        {
            Allocator::operator=(rhs);
            realloc(rhs._size);
            memory::copy(_data, rhs._data, rhs._size);
            _size = rhs._size;
        }
    }
    return *this;
}
template <typename T, typename TS, typename Allocator>
inline ArrayMemory<T, TS, Allocator>& ArrayMemory<T, TS, Allocator>::operator=(ArrayMemory&& rhs) noexcept
{
    if (this != &rhs)
    {
        // clear
        free();

        // move data
        Allocator::operator=(std::move(rhs));
        _data     = rhs._data;
        _size     = rhs._size;
        _capacity = rhs._capacity;

        rhs._data     = nullptr;
        rhs._size     = 0;
        rhs._capacity = 0;
    }
    return *this;
}

// memory operations
template <typename T, typename TS, typename Allocator>
inline void ArrayMemory<T, TS, Allocator>::realloc(SizeType new_capacity) noexcept
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
template <typename T, typename TS, typename Allocator>
inline void ArrayMemory<T, TS, Allocator>::free() noexcept
{
    if (_data)
    {
        Allocator::template free<T>(_data);
        _data     = nullptr;
        _size     = 0;
        _capacity = 0;
    }
}
template <typename T, typename TS, typename Allocator>
inline void ArrayMemory<T, TS, Allocator>::grow(SizeType new_size) noexcept
{
    SKR_ASSERT(new_size > _size);

    SizeType new_capacity = default_get_grow<T>(new_size, _capacity);
    SKR_ASSERT(new_capacity >= _capacity);
    if (new_capacity >= _capacity)
    {
        realloc(new_capacity);
    }
}
template <typename T, typename TS, typename Allocator>
inline void ArrayMemory<T, TS, Allocator>::shrink() noexcept
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
template <typename T, typename TS, typename Allocator>
inline T* ArrayMemory<T, TS, Allocator>::data() noexcept
{
    return _data;
}
template <typename T, typename TS, typename Allocator>
inline const T* ArrayMemory<T, TS, Allocator>::data() const noexcept
{
    return _data;
}
template <typename T, typename TS, typename Allocator>
inline typename ArrayMemory<T, TS, Allocator>::SizeType ArrayMemory<T, TS, Allocator>::size() const noexcept
{
    return _size;
}
template <typename T, typename TS, typename Allocator>
inline typename ArrayMemory<T, TS, Allocator>::SizeType ArrayMemory<T, TS, Allocator>::capacity() const noexcept
{
    return _capacity;
}

// setter
template <typename T, typename TS, typename Allocator>
inline void ArrayMemory<T, TS, Allocator>::set_size(SizeType new_size) noexcept
{
    _size = new_size;
}
} // namespace skr::container

// fixed array memory
namespace skr::container
{
// ctor & dtor
template <typename T, typename TS, uint64_t kCount>
inline FixedArrayMemory<T, TS, kCount>::FixedArrayMemory(AllocatorCtorParam) noexcept
{
}
template <typename T, typename TS, uint64_t kCount>
inline FixedArrayMemory<T, TS, kCount>::~FixedArrayMemory() noexcept
{
    free();
}

// copy & move
template <typename T, typename TS, uint64_t kCount>
inline FixedArrayMemory<T, TS, kCount>::FixedArrayMemory(const FixedArrayMemory& other, AllocatorCtorParam) noexcept
{
    if (other._size > 0)
    {
        memory::copy(data(), other.data(), other._size);
        _size = other._size;
    }
}
template <typename T, typename TS, uint64_t kCount>
inline FixedArrayMemory<T, TS, kCount>::FixedArrayMemory(FixedArrayMemory&& other) noexcept
{
    if (other._size > 0)
    {
        memory::move(data(), other.data(), other._size);
        _size = other._size;

        other._size = 0;
    }
}

// assign & move assign
template <typename T, typename TS, uint64_t kCount>
inline FixedArrayMemory<T, TS, kCount>& FixedArrayMemory<T, TS, kCount>::operator=(const FixedArrayMemory& rhs) noexcept
{
    if (this != &rhs)
    {
        // clear
        free();

        // copy data
        if (rhs._size > 0)
        {
            memory::copy(data(), rhs.data(), rhs._size);
            _size = rhs._size;
        }
    }
    return *this;
}
template <typename T, typename TS, uint64_t kCount>
inline FixedArrayMemory<T, TS, kCount>& FixedArrayMemory<T, TS, kCount>::operator=(FixedArrayMemory&& rhs) noexcept
{
    if (this != &rhs)
    {
        // clear
        free();

        // move data
        if (rhs._size > 0)
        {
            memory::move(data(), rhs.data(), rhs._size);
            _size = rhs._size;

            rhs._size = 0;
        }
    }
    return *this;
}

// memory operations
template <typename T, typename TS, uint64_t kCount>
inline void FixedArrayMemory<T, TS, kCount>::realloc(SizeType new_capacity) noexcept
{
    SKR_ASSERT(new_capacity <= kCount && "FixedArrayMemory can't alloc memory that larger than kCount");
}
template <typename T, typename TS, uint64_t kCount>
inline void FixedArrayMemory<T, TS, kCount>::free() noexcept
{
    if (_size)
    {
        memory::destruct(data(), _size);
        _size = 0;
    }
}
template <typename T, typename TS, uint64_t kCount>
inline void FixedArrayMemory<T, TS, kCount>::grow(SizeType new_size) noexcept
{
    SKR_ASSERT(new_size <= kCount && "FixedArrayMemory can't alloc memory that larger than kCount");
    // do noting
}
template <typename T, typename TS, uint64_t kCount>
inline void FixedArrayMemory<T, TS, kCount>::shrink() noexcept
{
    // do noting
}

// getter
template <typename T, typename TS, uint64_t kCount>
inline T* FixedArrayMemory<T, TS, kCount>::data() noexcept
{
    return reinterpret_cast<T*>(_placeholder.storage);
}
template <typename T, typename TS, uint64_t kCount>
inline const T* FixedArrayMemory<T, TS, kCount>::data() const noexcept
{
    return reinterpret_cast<const T*>(_placeholder.storage);
}
template <typename T, typename TS, uint64_t kCount>
inline typename FixedArrayMemory<T, TS, kCount>::SizeType FixedArrayMemory<T, TS, kCount>::size() const noexcept
{
    return _size;
}
template <typename T, typename TS, uint64_t kCount>
inline typename FixedArrayMemory<T, TS, kCount>::SizeType FixedArrayMemory<T, TS, kCount>::capacity() const noexcept
{
    return kCount;
}

// setter
template <typename T, typename TS, uint64_t kCount>
inline void FixedArrayMemory<T, TS, kCount>::set_size(SizeType new_size) noexcept
{
    _size = new_size;
}
} // namespace skr::container