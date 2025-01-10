#pragma once
#include "SkrBase/config.h"
#include "SkrBase/misc/integer_tools.hpp"
#include "SkrBase/memory.hpp"
#include "SkrBase/misc/debug.h"
#include "SkrBase/containers/misc/default_capicity_policy.hpp"

// TODO. literal 操作 API
//  1. literal 与 COW 视为同类，采取主动标记（set_literal）和主动触发 copy（pre_modify）来切换形态
// Base 自带 SSO 和常量优化，且支持三形态切换
//  [literal]==pre_modify()==>[heap]/[sso]
//
//  [sso]===realloc()==>[heap]
//       |==set_literal()==>[literal]
//
//  [heap]===realloc()==>[sso]
//        |==set_literal()==>[literal]
namespace skr::container
{
template <typename TS, uint64_t SSOSize = 31>
struct StringMemoryBase {
    static constexpr uint64_t SSOBufferSize = SSOSize + 1;
    static constexpr uint64_t SSOCapacity   = SSOSize - 1;

    static_assert(SSOBufferSize % 4 == 0, "SSOSize must be 4n - 2");
    static_assert(SSOBufferSize > sizeof(TS) * 2 + sizeof(void*), "SSOSize must be larger than heap data size");
    // uint8_t max (used to store sso string size)
    static_assert(SSOBufferSize < 128, "SSOBufferSize must be less than 127");

    using SizeType = TS;

    // getter
    inline SizeType size() const noexcept { return _is_sso() ? _sso_size : _size; }
    inline SizeType capacity() const noexcept
    {
        return is_literal() ? _size :
               _is_sso()    ? SSOCapacity :
                              _capacity;
    }

    // string literals
    inline bool is_literal() const noexcept { return !_is_sso() && _size >= 0 && _capacity == 0; }
    inline bool is_sso() const noexcept { return _is_sso(); }
    inline bool is_heap() const noexcept { return !_is_sso() && _capacity > 0; }

protected:
    // data getter
    inline void*       _raw_data() noexcept { return _is_sso() ? _sso_data : _data; }
    inline const void* _raw_data() const noexcept { return _is_sso() ? _sso_data : _data; }

    // sso
    inline void _reset_sso() noexcept
    {
        memset(_sso_buffer, 0, SSOBufferSize);
        _sso_flag = 1;
    }
    inline void _reset_heap() noexcept
    {
        _data     = nullptr;
        _size     = 0;
        _capacity = 0;
        _sso_flag = 0;
    }
    inline void _reset_literal(const void* data, SizeType size) noexcept
    {
        _data     = const_cast<void*>(data);
        _size     = size;
        _capacity = 0;
        _sso_flag = 0;
    }
    inline bool _is_sso() const noexcept { return _sso_flag; }

protected:
    union
    {
        struct {
            void*    _data;
            SizeType _size;
            SizeType _capacity;
        };
        struct {
            uint8_t _sso_data[SSOSize];
            uint8_t _sso_flag : 1;
            uint8_t _sso_size : 7;
        };
        uint8_t _sso_buffer[SSOBufferSize];
    };
};

// SSO string memory
// TODO. copy 考虑 literal
template <typename T, typename TS, uint64_t SSOSize, typename Allocator>
struct StringMemory : public StringMemoryBase<TS, SSOSize>, public Allocator {
    using Base               = StringMemoryBase<TS, SSOSize>;
    using DataType           = T;
    using SizeType           = typename Base::SizeType;
    using AllocatorCtorParam = typename Allocator::CtorParam;
    using Base::SSOBufferSize;
    using Base::SSOCapacity;

    // ctor & dtor
    inline StringMemory(AllocatorCtorParam param) noexcept
        : Base()
        , Allocator(std::move(param))
    {
        Base::_reset_sso();
    }
    inline ~StringMemory() noexcept
    {
        clear();
        free();
    }

    // copy & move
    inline StringMemory(const StringMemory& rhs) noexcept
        : Base()
        , Allocator(rhs)
    {
        if (rhs.is_literal())
        {
            Base::_reset_literal(rhs.data(), rhs.size());
        }
        else if (rhs.size())
        {
            Base::_reset_sso();
            realloc(rhs.size());
            memory::copy(data(), rhs.data(), rhs.size());
            set_size(rhs.size());
        }
        else
        {
            Base::_reset_sso();
        }
    }
    inline StringMemory(StringMemory&& rhs) noexcept
        : Base()
        , Allocator(std::move(rhs))
    {
        if (rhs.is_literal())
        {
            Base::_reset_literal(rhs.data(), rhs.size());
        }
        else if (rhs._is_sso())
        {
            Base::_reset_sso();
            memory::move(sso_data(), rhs.sso_data(), rhs._sso_size + 1); // include '\0
            Base::_sso_size = rhs._sso_size;
        }
        else
        {
            Base::_reset_heap();
            Base::_data     = rhs._data;
            Base::_size     = rhs._size;
            Base::_capacity = rhs._capacity;
        }

        // reset rhs
        rhs._reset_sso();
    }

    // assign & move assign
    inline void operator=(const StringMemory& rhs) noexcept
    {
        if (this != &rhs)
        {
            // copy allocator
            Allocator::operator=(rhs);

            // clean up self
            clear();

            // copy data
            if (rhs.is_literal())
            {
                if (!Base::is_literal())
                {
                    free();
                }
                Base::_reset_literal(rhs.data(), rhs.size());
            }
            else if (rhs.size() > 0)
            {
                // reserve memory
                if (Base::capacity() < rhs.size())
                {
                    realloc(rhs.size());
                }

                // copy data
                memory::copy(data(), rhs.data(), rhs.size());
                set_size(rhs.size());
            }
        }
    }
    inline void operator=(StringMemory&& rhs) noexcept
    {
        if (this != &rhs)
        {
            // move allocator
            Allocator::operator=(std::move(rhs));

            // clean up self
            clear();
            free();

            // move data
            if (rhs.is_literal())
            {
                if (!Base::is_literal())
                {
                    free();
                }
                Base::_reset_literal(rhs.data(), rhs.size());
            }
            else if (rhs._is_sso())
            {
                Base::_reset_sso();
                memory::move(sso_data(), rhs.sso_data(), rhs._sso_size + 1); // include '\0'
                Base::_sso_size = rhs._sso_size;
            }
            else
            {
                Base::_reset_heap();
                Base::_data     = rhs._data;
                Base::_size     = rhs._size;
                Base::_capacity = rhs._capacity;
            }

            // reset rhs
            rhs._reset_sso();
        }
    }

    // setter
    inline void set_size(SizeType value) noexcept
    {
        SKR_ASSERT(value <= Base::capacity() && "size must be less than capacity");
        SKR_ASSERT(!Base::is_literal() && "literal state must be canceled before size change");

        if (Base::_is_sso())
        {
            Base::_sso_size = value;
            // trigger '\0' update
            sso_data()[Base::_sso_size] = 0;
        }
        else
        {
            Base::_size = value;
            // trigger '\0' update
            heap_data()[Base::_size] = 0;
        }
    }

    // memory operations
    inline void realloc(SizeType new_capacity) noexcept
    {
        SKR_ASSERT(!Base::is_literal() && "literal state must be canceled before memory ops");

        SKR_ASSERT(new_capacity != Base::_capacity);
        SKR_ASSERT(new_capacity > 0);
        SKR_ASSERT(Base::size() <= new_capacity);

        new_capacity = new_capacity < SSOCapacity ? SSOCapacity : new_capacity;

        // update data
        if (new_capacity > SSOCapacity)
        {
            if (Base::_is_sso()) // inline -> heap
            {
                SizeType data_size = Base::_sso_size;

                // alloc new memory
                DataType* new_memory = Allocator::template alloc<DataType>(new_capacity + 1); // include '\0'

                // move items
                if (data_size)
                {
                    memory::move(new_memory, sso_data(), data_size + 1); // include '\0'
                }

                // rebuild heap data
                Base::_reset_heap();
                Base::_data     = new_memory;
                Base::_size     = data_size;
                Base::_capacity = new_capacity;
            }
            else // heap -> heap
            {
                // alloc new memory
                DataType* new_memory = Allocator::template alloc<DataType>(new_capacity + 1); // include '\0'

                // move items
                if (Base::_size)
                {
                    memory::move(new_memory, heap_data(), Base::_size + 1); // include '\0'
                }

                // release old memory
                Allocator::template free<DataType>(heap_data());

                // update data
                Base::_data = new_memory;

                // update capacity
                Base::_capacity = new_capacity;
            }
        }
        else
        {
            if (Base::_is_sso()) // inline -> inline
            {
                // do nothing
            }
            else // heap -> inline
            {
                DataType* cached_heap_data = data();
                SizeType  cached_heap_size = Base::_size;

                // move items
                Base::_reset_sso();
                if (cached_heap_size)
                {
                    memory::move(sso_data(), cached_heap_data, cached_heap_size + 1); // include '\0'
                    Base::_sso_size = cached_heap_size;
                }

                // release old memory
                Allocator::template free<DataType>(cached_heap_data);
            }
        }
    }
    inline void free() noexcept
    {
        SKR_ASSERT(!Base::is_literal() && "literal state must be canceled before realloc");

        if (!Base::_is_sso())
        {
            Allocator::template free<DataType>(heap_data());
            Base::_reset_sso();
        }
    }
    inline SizeType grow(SizeType grow_size) noexcept
    {
        SKR_ASSERT(!Base::is_literal() && "literal state must be canceled before realloc");

        SizeType old_size = Base::size();
        SizeType new_size = old_size + grow_size;

        if (new_size > Base::capacity())
        {
            SizeType new_capacity = default_get_grow<DataType>(new_size, Base::capacity());
            SKR_ASSERT(new_capacity >= Base::capacity());
            if (new_capacity >= Base::capacity())
            {
                realloc(new_capacity);
            }
        }

        set_size(new_size);
        return old_size;
    }
    inline void shrink() noexcept
    {
        SKR_ASSERT(!Base::is_literal() && "literal state must be canceled before realloc");

        SizeType new_capacity = default_get_shrink<DataType>(Base::size(), Base::capacity());
        SKR_ASSERT(new_capacity >= Base::size());
        if (new_capacity < Base::capacity())
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
        if (Base::is_literal())
        {
            Base::_reset_sso();
        }
        else if (Base::size())
        {
            memory::destruct(data(), Base::size());
            set_size(0);
        }
    }

    // literal
    inline void set_literal(const DataType* data, SizeType len) noexcept
    {
        // clean up
        clear();
        free();

        // setup data
        Base::_reset_literal(data, len);
    }
    inline bool pre_modify(SizeType except_size = 0) noexcept
    {
        // if you want to clean up literal data, use clear() instead
        if (Base::is_literal())
        {
            // cache data
            DataType* literal_data = heap_data();
            SizeType  literal_size = Base::_size;

            // reset to empty string
            Base::_reset_sso();

            // realloc memory
            except_size = except_size ? except_size : literal_size;
            realloc(except_size);

            // copy data
            SizeType copy_size = std::min(except_size, literal_size);
            memory::copy(data(), literal_data, copy_size);
            set_size(copy_size);

            return true;
        }
        return false;
    }

    // getter
    inline DataType*       data() noexcept { return reinterpret_cast<DataType*>(Base::_raw_data()); }
    inline const DataType* data() const noexcept { return reinterpret_cast<const DataType*>(Base::_raw_data()); }

private:
    // helper
    inline DataType*       sso_data() noexcept { return reinterpret_cast<DataType*>(Base::_sso_data); }
    inline const DataType* sso_data() const noexcept { return reinterpret_cast<DataType*>(Base::_sso_data); }
    inline DataType*       heap_data() noexcept { return reinterpret_cast<DataType*>(Base::_data); }
    inline const DataType* heap_data() const noexcept { return reinterpret_cast<DataType*>(Base::_data); }
};

// COW string memory
struct COWStringMemory {
};

// serde string memory
struct SerdeStringMemory {
};

} // namespace skr::container
