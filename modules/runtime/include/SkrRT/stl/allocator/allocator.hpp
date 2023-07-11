#pragma once
#include "SkrRT/base/config.hpp"
#include "arena.hpp"
#include <limits>
#include "SkrRT/base/memory.hpp"
#include "SkrRT/base/tools/assert.hpp"

namespace skr
{
template <typename TDerived, typename TS>
class AllocTemplate
{
public:
    using SizeType = TS;

    // impl it
    // SKR_INLINE void  freeRaw(void* p, SizeType align);
    // SKR_INLINE void* allocRaw(SizeType size, SizeType align);
    // SKR_INLINE void* reallocRaw(void* p, SizeType size, SizeType align);

    // helper
    template <typename T>
    SKR_INLINE void free(T* p) { static_cast<TDerived*>(this)->freeRaw(p, alignof(T)); }
    template <typename T>
    SKR_INLINE T* alloc(SizeType size) { return (T*)static_cast<TDerived*>(this)->allocRaw(size * sizeof(T), alignof(T)); }
    template <typename T>
    SKR_INLINE T* realloc(T* p, SizeType size)
    {
        return (T*)static_cast<TDerived*>(this)->reallocRaw(p, size * sizeof(T), alignof(T));
    }

    // size > capacity, calc grow
    SKR_INLINE SizeType getGrow(SizeType size, SizeType capacity)
    {
        constexpr SizeType first_grow    = 4;
        constexpr SizeType constant_grow = 16;

        SKR_Assert(size > capacity && size > 0);

        // init data
        SizeType result = first_grow;

        // calc grow
        if (capacity || size > first_grow)
        {
            result = size + 3 * size / 8 + constant_grow;
        }

        // handle num over flow
        if (size > result)
            result = std::numeric_limits<SizeType>::max();

        return result;
    }
    // size < capacity, calc shrink
    SKR_INLINE SizeType getShrink(SizeType size, SizeType capacity)
    {
        SKR_Assert(size < capacity);

        SizeType result;
        if ((3 * size < 2 * capacity) && (capacity - size > 64 || !size))
        {
            result = size;
        }
        else
        {
            result = capacity;
        }

        return result;
    }

    // resize helper
    template <typename T>
    SKR_INLINE T* resizeContainer(T* p, SizeType size, SizeType capacity, SizeType new_capacity)
    {
        SKR_Assert(new_capacity > 0);
        SKR_Assert(size <= capacity);
        SKR_Assert((capacity > 0 && p != nullptr) || (capacity == 0 && p == nullptr));

        if constexpr (memory::memory_traits<T>::use_realloc)
        {
            return realloc(p, new_capacity);
        }

        // alloc new memory
        T* new_memory = alloc<T>(new_capacity);

        // move memory
        if (size)
        {
            // move items
            memory::move(new_memory, p, std::min(size, new_capacity));

            // destruct old items
            memory::destruct(p, size);

            // release old memory
            free(p);
        }

        return new_memory;
    }
};
} // namespace skr

namespace skr
{
class PmrAllocator : public AllocTemplate<PmrAllocator, size_t>
{
public:
    // ctor...
    SKR_INLINE PmrAllocator(IArena* res = default_arena())
        : _arena(res)
    {
        SKR_ASSERT(_arena != nullptr);
    }
    SKR_INLINE               PmrAllocator(const PmrAllocator&) = default;
    SKR_INLINE               PmrAllocator(PmrAllocator&&)      = default;
    SKR_INLINE PmrAllocator& operator=(const PmrAllocator&)    = default;
    SKR_INLINE PmrAllocator& operator=(PmrAllocator&&)         = default;

    // impl
    SKR_INLINE void  freeRaw(void* p, SizeType align) { _arena->free(p); }
    SKR_INLINE void* allocRaw(SizeType size, SizeType align) { return _arena->alloc(size, align); }
    SKR_INLINE void* reallocRaw(void* p, SizeType size, SizeType align) { return _arena->realloc(p, size, align); }

private:
    IArena* _arena;
};
} // namespace skr