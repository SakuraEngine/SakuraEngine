#pragma once
#include "SkrBase/config.h"
#include "SkrBase/misc/debug.h"
#include "SkrBase/containers/misc/placeholder.hpp"
#include "SkrBase/containers/misc/default_capicity_policy.hpp"
#include "SkrBase/memory/memory_ops.hpp"

// generic vector fwd
namespace skr
{
struct GenericVector;
}

// vector memory base
namespace skr::container
{
template <typename TSize>
struct VectorMemoryBase {
    using SizeType = TSize;

    friend struct ::skr::GenericVector;

    // getter
    inline SizeType size() const noexcept { return _size; }
    inline SizeType capacity() const noexcept { return _capacity; }

    // setter
    inline void set_size(SizeType value) noexcept { _size = value; }

private:
    // helper for generic
    inline void* _item_at(SizeType index, SizeType item_size) const
    {
        return ::skr::memory::offset_item(_data, item_size, index);
    }
    inline static VectorMemoryBase* _memory_at(void* p, SizeType index)
    {
        SKR_ASSERT(p);
        return reinterpret_cast<VectorMemoryBase*>(
            ::skr::memory::offset_item(p, sizeof(VectorMemoryBase), index)
        );
    }
    inline static const VectorMemoryBase* _memory_at(const void* p, SizeType index)
    {
        SKR_ASSERT(p);
        return reinterpret_cast<const VectorMemoryBase*>(
            ::skr::memory::offset_item(p, sizeof(VectorMemoryBase), index)
        );
    }

protected:
    void*    _data     = nullptr;
    SizeType _size     = 0;
    SizeType _capacity = 0;
};
} // namespace skr::container

// util vector memory
namespace skr::container
{
template <typename T, typename Base, typename Allocator>
struct VectorMemory : public Base, public Allocator {
    using DataType           = T;
    using SizeType           = typename Base::SizeType;
    using AllocatorCtorParam = typename Allocator::CtorParam;

    // ctor & dtor
    inline VectorMemory(AllocatorCtorParam param) noexcept
        : Base()
        , Allocator(std::move(param))
    {
    }
    inline ~VectorMemory() noexcept
    {
        clear();
        free();
    }

    // copy & move
    inline VectorMemory(const VectorMemory& rhs) noexcept
        : Base()
        , Allocator(rhs)
    {
        if (rhs._size)
        {
            realloc(rhs._size);
            ::skr::memory::copy(data(), rhs.data(), rhs._size);
            Base::_size = rhs._size;
        }
    }
    inline VectorMemory(VectorMemory&& rhs) noexcept
        : Base(std::move(rhs))
        , Allocator(std::move(rhs))
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
                if (Base::_capacity < rhs._size)
                {
                    realloc(rhs._size);
                }

                // copy data
                ::skr::memory::copy(data(), rhs.data(), rhs._size);
                Base::_size = rhs._size;
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
            Base::_data     = rhs._data;
            Base::_size     = rhs._size;
            Base::_capacity = rhs._capacity;

            // clean up rhs
            rhs._reset();
        }
    }

    // memory operations
    inline void realloc(SizeType new_capacity) noexcept
    {
        SKR_ASSERT(new_capacity != Base::_capacity);
        SKR_ASSERT(new_capacity > 0);
        SKR_ASSERT(Base::_size <= new_capacity);
        SKR_ASSERT((Base::_capacity > 0 && Base::_data != nullptr) || (Base::_capacity == 0 && Base::_data == nullptr));

        // update memory
        if constexpr (::skr::memory::MemoryTraits<DataType>::use_realloc && Allocator::support_realloc)
        {
            Base::_data = Allocator::template realloc<DataType>(data(), new_capacity);
        }
        else
        {
            // alloc new memory
            DataType* new_memory = Allocator::template alloc<DataType>(new_capacity);

            // move items
            if (Base::_size)
            {
                ::skr::memory::move(new_memory, data(), Base::_size);
            }

            // release old memory
            Allocator::template free<DataType>(data());

            // update data
            Base::_data = new_memory;
        }

        // update capacity
        Base::_capacity = new_capacity;
    }
    inline void free() noexcept
    {
        if (data())
        {
            Allocator::template free<DataType>(data());
            Base::_data     = nullptr;
            Base::_capacity = 0;
        }
    }
    inline SizeType grow(SizeType grow_size) noexcept
    {
        SizeType old_size = Base::_size;
        SizeType new_size = old_size + grow_size;

        if (new_size > Base::_capacity)
        {
            SizeType new_capacity = default_get_grow<DataType>(new_size, Base::_capacity);
            SKR_ASSERT(new_capacity >= Base::_capacity);
            if (new_capacity >= Base::_capacity)
            {
                realloc(new_capacity);
            }
        }

        Base::_size = new_size;
        return old_size;
    }
    inline void shrink() noexcept
    {
        SizeType new_capacity = default_get_shrink<DataType>(Base::_size, Base::_capacity);
        SKR_ASSERT(new_capacity >= Base::_size);
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
        if (Base::_size)
        {
            ::skr::memory::destruct(data(), Base::_size);
            Base::_size = 0;
        }
    }

    // getter
    inline DataType*       data() noexcept { return reinterpret_cast<DataType*>(Base::_data); }
    inline const DataType* data() const noexcept { return reinterpret_cast<const DataType*>(Base::_data); }

private:
    // helper functions
    inline void _reset() noexcept
    {
        Base::_data     = nullptr;
        Base::_size     = 0;
        Base::_capacity = 0;
    }
};
} // namespace skr::container

// fixed vector memory
namespace skr::container
{
template <typename T, uint64_t kCount, typename Base>
struct FixedVectorMemory : public Base {
    static_assert(kCount > 0, "FixedVectorMemory must have a capacity larger than 0");
    struct DummyParam {
    };
    using DataType           = T;
    using SizeType           = typename Base::SizeType;
    using AllocatorCtorParam = DummyParam;

    // ctor & dtor
    inline FixedVectorMemory(AllocatorCtorParam) noexcept
    {
        _init_setup();
    }
    inline ~FixedVectorMemory() noexcept
    {
        clear();
        free();
    }

    // copy & move
    inline FixedVectorMemory(const FixedVectorMemory& other) noexcept
    {
        _init_setup();

        if (other._size)
        {
            ::skr::memory::copy(data(), other.data(), other._size);
            Base::_size = other._size;
        }
    }
    inline FixedVectorMemory(FixedVectorMemory&& other) noexcept
    {
        _init_setup();

        if (other._size)
        {
            ::skr::memory::move(data(), other.data(), other._size);
            Base::_size = other._size;

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
                ::skr::memory::copy(data(), rhs.data(), rhs._size);
                Base::_size = rhs._size;
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
                ::skr::memory::move(data(), rhs.data(), rhs._size);
                Base::_size = rhs._size;

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
        SKR_ASSERT((Base::_size + grow_size) <= kCount && "FixedVectorMemory can't alloc memory that larger than kCount");

        SizeType old_size = Base::_size;
        Base::_size += grow_size;
        return old_size;
    }
    inline void shrink() noexcept
    {
        // do noting
    }
    inline void clear() noexcept
    {
        if (Base::_size)
        {
            ::skr::memory::destruct(data(), Base::_size);
            Base::_size = 0;
        }
    }

    // getter
    inline DataType*       data() noexcept { return reinterpret_cast<DataType*>(Base::_data); }
    inline const DataType* data() const noexcept { return reinterpret_cast<const DataType*>(Base::_data); }

private:
    inline void _init_setup() noexcept
    {
        Base::_data     = _placeholder.data_typed();
        Base::_capacity = kCount;
    }
    inline void _reset() noexcept
    {
        Base::_size = 0;
    }

private:
    Placeholder<DataType, kCount> _placeholder;
};
} // namespace skr::container

// inline vector memory
namespace skr::container
{
template <typename T, uint64_t kInlineCount, typename Base, typename Allocator>
struct InlineVectorMemory : public Base, public Allocator {
    using DataType           = T;
    using SizeType           = typename Base::SizeType;
    using AllocatorCtorParam = typename Allocator::CtorParam;

    // ctor & dtor
    inline InlineVectorMemory(AllocatorCtorParam param) noexcept
        : Base()
        , Allocator(std::move(param))
    {
        _reset();
    }
    inline ~InlineVectorMemory() noexcept
    {
        clear();
        free();
    }

    // copy & move
    inline InlineVectorMemory(const InlineVectorMemory& rhs) noexcept
        : Base()
        , Allocator(rhs)
    {
        _reset();

        if (rhs._size)
        {
            realloc(rhs._size);
            ::skr::memory::copy(data(), rhs.data(), rhs._size);
            Base::_size = rhs._size;
        }
    }
    inline InlineVectorMemory(InlineVectorMemory&& rhs) noexcept
        : Base()
        , Allocator(std::move(rhs))
    {
        _reset();

        // move data
        if (rhs._is_using_inline_memory())
        {
            ::skr::memory::move(_placeholder.data_typed(), rhs._placeholder.data_typed(), rhs._size);
            Base::_size = rhs._size;
        }
        else
        {
            Base::_data     = rhs._data;
            Base::_size     = rhs._size;
            Base::_capacity = rhs._capacity;
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
                if (Base::_capacity < rhs._size)
                {
                    realloc(rhs._size);
                }

                // copy data
                ::skr::memory::copy(data(), rhs.data(), rhs._size);
                Base::_size = rhs._size;
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
                ::skr::memory::move(data(), rhs.data(), rhs._size);
                Base::_size = rhs._size;
            }
            else
            {
                Base::_data     = rhs._data;
                Base::_size     = rhs._size;
                Base::_capacity = rhs._capacity;
            }

            // reset rhs
            rhs._reset();
        }
    }

    // memory operations
    inline void realloc(SizeType new_capacity) noexcept
    {
        SKR_ASSERT(new_capacity != Base::_capacity || Base::_capacity <= kInlineCount);
        SKR_ASSERT(new_capacity > 0);
        SKR_ASSERT(Base::_size <= new_capacity);
        new_capacity = new_capacity < kInlineCount ? kInlineCount : new_capacity;

        // update data
        if (new_capacity > kInlineCount)
        {
            if (_is_using_inline_memory()) // inline -> heap
            {
                // alloc new memory
                DataType* new_memory = Allocator::template alloc<DataType>(new_capacity);

                // move items
                if (Base::_size)
                {
                    ::skr::memory::move(new_memory, _placeholder.data_typed(), Base::_size);
                }

                // update data
                Base::_data = new_memory;
            }
            else // heap -> heap
            {
                if constexpr (::skr::memory::MemoryTraits<DataType>::use_realloc && Allocator::support_realloc)
                {
                    Base::_data     = Allocator::template realloc<DataType>(data(), new_capacity);
                    Base::_capacity = new_capacity;
                }
                else
                {
                    // alloc new memory
                    DataType* new_memory = Allocator::template alloc<DataType>(new_capacity);

                    // move items
                    if (Base::_size)
                    {
                        ::skr::memory::move(new_memory, data(), Base::_size);
                    }

                    // release old memory
                    Allocator::template free<DataType>(data());

                    // update data
                    Base::_data = new_memory;
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
                DataType* cached_heap_data = data();

                // move items
                if (Base::_size)
                {
                    ::skr::memory::move(_placeholder.data_typed(), cached_heap_data, Base::_size);
                }

                // release old memory
                Allocator::template free<DataType>(cached_heap_data);

                // reset data
                Base::_data = _placeholder.data_typed();
            }
        }

        // update capacity
        Base::_capacity = new_capacity;
    }
    inline void free() noexcept
    {
        if (!_is_using_inline_memory())
        {
            Allocator::template free<DataType>(data());
            _reset();
        }
    }
    inline SizeType grow(SizeType grow_size) noexcept
    {
        SizeType old_size = Base::_size;
        SizeType new_size = old_size + grow_size;

        if (new_size > Base::_capacity)
        {
            SizeType new_capacity = default_get_grow<DataType>(new_size, Base::_capacity);
            SKR_ASSERT(new_capacity >= Base::_capacity);
            if (new_capacity >= Base::_capacity)
            {
                realloc(new_capacity);
            }
        }

        Base::_size = new_size;
        return old_size;
    }
    inline void shrink() noexcept
    {
        SizeType new_capacity = default_get_shrink<DataType>(Base::_size, Base::_capacity);
        SKR_ASSERT(new_capacity >= Base::_size);
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
        if (Base::_size)
        {
            ::skr::memory::destruct(data(), Base::_size);
            Base::_size = 0;
        }
    }

    // getter
    inline DataType*       data() noexcept { return reinterpret_cast<DataType*>(Base::_data); }
    inline const DataType* data() const noexcept { return reinterpret_cast<const DataType*>(Base::_data); }

private:
    // helper
    inline bool _is_using_inline_memory() const noexcept { return Base::_data == _placeholder.data_typed(); }
    inline void _reset()
    {
        Base::_data     = _placeholder.data_typed();
        Base::_size     = 0;
        Base::_capacity = kInlineCount;
    }

private:
    Placeholder<DataType, kInlineCount> _placeholder;
};
} // namespace skr::container