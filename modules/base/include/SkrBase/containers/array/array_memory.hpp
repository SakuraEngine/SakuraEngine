#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/fwd_container.hpp"
#include "SkrBase/misc/debug.h"
#include "SkrBase/containers/misc/placeholder.hpp"
#include "SkrBase/containers/allocator/allocator.hpp"

namespace skr::container
{
// TODO. 此处不如参考 EASTL 的 VectorBase，将内存相关的操作直接封装到 Memory 系统内部去，这样可以天然避免代码的割裂
// 从数据层面，Memory 需要记录所有的数据，这样，表层的 Allocator 就成为了只需要关注算法调度执行的外层，同时，这也是无可奈何的，因为 Copy 和 Move 需要知悉所有的情况
// ArrayMemory 提供只与内存相关联的最简化 API：
//      ctor、dtor、copy、move、assign、move assign
//      _realloc
//      _free
//      _grow (或 get_grow)
//      _shrink (或 get_shrink)
template <typename T, typename TS, typename HeapAllocator>
struct ArrayMemory : private HeapAllocator {
    // configure
    using SizeType                           = TS;
    using CtorParam                          = typename HeapAllocator::CtorParam;
    static constexpr bool with_inline_memory = false;

    // ctor
    ArrayMemory() requires(std::same_as<CtorParam, void>);
    ArrayMemory(CtorParam param = {}) requires(!std::same_as<CtorParam, void>);

    // copy & move
    ArrayMemory(const ArrayMemory& other) requires(std::same_as<CtorParam, void>);
    ArrayMemory(const ArrayMemory& other, CtorParam param = {}) requires(!std::same_as<CtorParam, void>);
    ArrayMemory(ArrayMemory&& other);

    // assign & move assign
    ArrayMemory& operator=(const ArrayMemory& other);
    ArrayMemory& operator=(ArrayMemory&& other);

    // grow/shrink policy
    SizeType get_grow(SizeType expect_size, SizeType capacity) const;
    SizeType get_shrink(SizeType expect_size, SizeType capacity) const;

    // alloc policy
    void free();
    template <bool kDesiredRealloc, typename MoveFunc>
    void realloc(SizeType new_size, MoveFunc&& move_func); // move func: void(T* new_memory, T* old_memory);

    // inline memory (need copy data when move)
    bool is_using_inline_memory() const;

    // getter
    T*       data();
    const T* data() const;
    SizeType capacity() const;

private:
    T*       _data     = nullptr;
    SizeType _capacity = 0;
};

template <typename T, typename TS, size_t kCount>
struct FixedArrayMemory {
    // configure
    using SizeType                           = TS;
    using CtorParam                          = void;
    static constexpr bool with_inline_memory = true;

    // ctor
    FixedArrayMemory();

    // copy & move
    FixedArrayMemory(const FixedArrayMemory& other);
    FixedArrayMemory(FixedArrayMemory&& other);

    // assign & move assign
    FixedArrayMemory& operator=(const FixedArrayMemory& other);
    FixedArrayMemory& operator=(FixedArrayMemory&& other);

    // grow/shrink policy
    SizeType get_grow(SizeType expect_size, SizeType capacity) const;
    SizeType get_shrink(SizeType expect_size, SizeType capacity) const;

    // alloc policy
    void free();
    template <bool kDesiredRealloc, typename MoveFunc>
    void realloc(SizeType new_size, MoveFunc&& move_func); // move func: void(T* new_memory, T* old_memory);

    // inline memory (need copy data when move)
    bool is_using_inline_memory() const;

    // getter
    T*       data();
    const T* data() const;
    SizeType capacity() const;

private:
    Placeholder<T, kCount> _placeholder;
};

template <typename T, typename TS, size_t kCOunt, typename HeapAllocator>
struct InlineArrayMemory {
    // configure
    using SizeType                           = TS;
    using CtorParam                          = typename HeapAllocator::CtorParam;
    static constexpr bool with_inline_memory = true;

    // ctor
    InlineArrayMemory();

    // copy & move
    InlineArrayMemory(const InlineArrayMemory& other);
    InlineArrayMemory(InlineArrayMemory&& other);

    // assign & move assign
    InlineArrayMemory& operator=(const InlineArrayMemory& other);
    InlineArrayMemory& operator=(InlineArrayMemory&& other);

    // grow/shrink policy
    SizeType get_grow(SizeType expect_size, SizeType capacity) const;
    SizeType get_shrink(SizeType expect_size, SizeType capacity) const;

    // alloc policy
    void free();
    template <bool kDesiredRealloc, typename MoveFunc>
    void realloc(SizeType new_size, MoveFunc&& move_func); // move func: void(T* new_memory, T* old_memory);

    // inline memory (need copy data when move)
    bool is_using_inline_memory() const;

    // getter
    T*       data();
    const T* data() const;
    SizeType capacity() const;

private:
    union
    {
        Placeholder<T, kCOunt> _placeholder;
        SizeType               _capacity;
    };
    T* _heap_data;
};
} // namespace skr::container

// common memory
namespace skr::container
{
// ctor
template <typename T, typename TS, typename HeapAllocator>
inline ArrayMemory<T, TS, HeapAllocator>::ArrayMemory() requires(std::same_as<CtorParam, void>) = default;
template <typename T, typename TS, typename HeapAllocator>
inline ArrayMemory<T, TS, HeapAllocator>::ArrayMemory(CtorParam param) requires(!std::same_as<CtorParam, void>)
    : HeapAllocator(std::move(param))
{
}

// copy & move
template <typename T, typename TS, typename HeapAllocator>
inline ArrayMemory<T, TS, HeapAllocator>::ArrayMemory(const ArrayMemory& other) requires(std::same_as<CtorParam, void>)
    : HeapAllocator()
{
    if (other._capacity > 0)
    {
        _data     = HeapAllocator::allocate(other._capacity);
        _capacity = other._capacity;
    }
}
template <typename T, typename TS, typename HeapAllocator>
inline ArrayMemory<T, TS, HeapAllocator>::ArrayMemory(const ArrayMemory& other, CtorParam param) requires(!std::same_as<CtorParam, void>)
    : HeapAllocator(std::move(param))
{
    if (other._capacity > 0)
    {
        _data     = HeapAllocator::allocate(other._capacity);
        _capacity = other._capacity;
    }
}
template <typename T, typename TS, typename HeapAllocator>
inline ArrayMemory<T, TS, HeapAllocator>::ArrayMemory(ArrayMemory&& other)
    : HeapAllocator(std::move(other))
{
    if (other._capacity > 0)
    {
        // move data
        _data     = other._data;
        _capacity = other._capacity;

        // invalidate other
        other._data     = nullptr;
        other._capacity = 0;
    }
}

// assign & move assign
template <typename T, typename TS, typename HeapAllocator>
inline ArrayMemory<T, TS, HeapAllocator>& ArrayMemory<T, TS, HeapAllocator>::operator=(const ArrayMemory& other)
{
    if (this != &other)
    {
        // free old memory
        free();

        // allocate new memory
        if (other._capacity > 0)
        {
            _data     = HeapAllocator::alloc(other._capacity * sizeof(T), alignof(T));
            _capacity = other._capacity;
        }
    }
    return *this;
}
template <typename T, typename TS, typename HeapAllocator>
inline ArrayMemory<T, TS, HeapAllocator>& ArrayMemory<T, TS, HeapAllocator>::operator=(ArrayMemory&& other)
{
    if (this != &other)
    {
        // free old memory
        free();

        // move data
        _data     = other._data;
        _capacity = other._capacity;

        // invalidate other
        other._data     = nullptr;
        other._capacity = 0;
    }
    return *this;
}

// grow/shrink policy
template <typename T, typename TS, typename HeapAllocator>
inline typename ArrayMemory<T, TS, HeapAllocator>::SizeType ArrayMemory<T, TS, HeapAllocator>::get_grow(SizeType expect_size, SizeType capacity) const
{
    default_get_grow<T>(expect_size, capacity);
}
template <typename T, typename TS, typename HeapAllocator>
inline typename ArrayMemory<T, TS, HeapAllocator>::SizeType ArrayMemory<T, TS, HeapAllocator>::get_shrink(SizeType expect_size, SizeType capacity) const
{
    default_get_shrink<T>(expect_size, capacity);
}

// alloc policy
template <typename T, typename TS, typename HeapAllocator>
inline void ArrayMemory<T, TS, HeapAllocator>::free()
{
    if (_data)
    {
        HeapAllocator::free(_data, alignof(T));
        _data     = nullptr;
        _capacity = 0;
    }
}
template <typename T, typename TS, typename HeapAllocator>
template <bool kDesiredRealloc, typename MoveFunc>
inline void ArrayMemory<T, TS, HeapAllocator>::realloc(SizeType new_size, MoveFunc&& move_func)
{
    if constexpr (kDesiredRealloc && HeapAllocator::support_realloc)
    {
        if (_data)
        {
            _data = HeapAllocator::realloc(_data, new_size * sizeof(T), alignof(T));
        }
        else
        {
            _data = HeapAllocator::alloc(new_size * sizeof(T), alignof(T));
        }
    }
    else
    {
        // allocate new memory
        T* new_memory = HeapAllocator::alloc(new_size * sizeof(T), alignof(T));

        // move data
        if (_data)
        {
            move_func(new_memory, _data);
        }

        // free old memory
        HeapAllocator::free(_data, alignof(T));

        // update data
        _data     = new_memory;
        _capacity = new_size;
    }
}

// inline memory (need copy data when move)
template <typename T, typename TS, typename HeapAllocator>
inline bool ArrayMemory<T, TS, HeapAllocator>::is_using_inline_memory() const
{
    return false;
}

// getter
template <typename T, typename TS, typename HeapAllocator>
inline T* ArrayMemory<T, TS, HeapAllocator>::data()
{
    return _data;
}
template <typename T, typename TS, typename HeapAllocator>
inline const T* ArrayMemory<T, TS, HeapAllocator>::data() const
{
    return _data;
}
template <typename T, typename TS, typename HeapAllocator>
inline typename ArrayMemory<T, TS, HeapAllocator>::SizeType ArrayMemory<T, TS, HeapAllocator>::capacity() const
{
    return _capacity;
}
} // namespace skr::container