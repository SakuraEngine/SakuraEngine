#pragma once
#include "SkrBase/config.h"
#include "SkrBase/misc/integer_tools.hpp"
#include "SkrBase/memory.hpp"
#include "SkrBase/misc/debug.h"

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
    inline SizeType capacity() const noexcept { return _is_sso() ? SSOCapacity : _capacity; }

    // setter
    inline void set_size(SizeType value) noexcept
    {
        if (_is_sso())
        {
            _sso_size = value;
            // trigger '\0' update
            _sso_data[_sso_size] = 0;
        }
        else
        {
            _size = value;
            // trigger '\0' update
            _data[_size] = 0;
        }
    }

    // string literals
    inline bool is_literal() const noexcept { return !is_sso() && _size > 0 && _capacity == 0; }

protected:
    // data getter
    inline void*       _raw_data() noexcept { return is_sso() ? _sso_data : _data; }
    inline const void* _raw_data() const noexcept { return is_sso() ? _sso_data : _data; }

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
    inline bool _is_sso() const noexcept { return _sso_flag; }

protected:
    union
    {
        struct {
            void*    _data     = nullptr;
            SizeType _size     = 0;
            SizeType _capacity = 0;
        };
        struct {
            uint8_t _sso_data[SSOSize];
            uint8_t _sso_flag : 1;
            uint8_t _sso_size;
        };
        uint8_t _sso_buffer[SSOBufferSize];
    };
};

// SSO string memory
template <typename T, typename TS, uint64_t SSOSize, typename Allocator>
struct StringMemory : public StringMemoryBase<TS, SSOSize>, public Allocator {
    using Base               = StringMemoryBase<TS, SSOSize>;
    using DataType           = T;
    using SizeType           = typename Base::SizeType;
    using AllocatorCtorParam = typename Allocator::CtorParam;
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
        Base::_reset_sso();

        if (rhs.size())
        {
            realloc(rhs.size());
            memory::copy(data(), rhs.data(), rhs.size());
            Base::set_size(rhs.size());
        }
    }
    inline StringMemory(StringMemory&& rhs) noexcept
        : Base()
        , Allocator(std::move(rhs))
    {
        // move data
        if (rhs._is_sso())
        {
            Base::_reset_sso();
            memory::move(Base::_sso_data, rhs._sso_data, rhs._sso_size + 1); // include '\0
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
            if (rhs.size() > 0)
            {
                // reserve memory
                if (Base::capacity() < rhs.size())
                {
                    realloc(rhs.size());
                }

                // copy data
                memory::copy(data(), rhs.data(), rhs.size());
                Base::set_size(rhs.size());
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
            if (rhs._is_sso())
            {
                Base::_reset_sso();
                memory::move(Base::_sso_data, rhs._sso_data, rhs._sso_size + 1); // include '\0'
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

    // memory operations
    inline void realloc(SizeType new_capacity) noexcept
    {
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
                    memory::move(new_memory, Base::_sso_data, data_size + 1); // include '\0'
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
                    memory::move(new_memory, Base::_data, Base::_size + 1); // include '\0'
                }

                // release old memory
                Allocator::template free<DataType>(Base::_data);

                // update data
                Base::_data = new_memory;
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
                    memory::move(Base::_sso_data, cached_heap_data, cached_heap_size + 1); // include '\0'
                    Base::_sso_size = cached_heap_size;
                }

                // release old memory
                Allocator::template free<DataType>(cached_heap_data);
            }
        }
    }
    inline void free() noexcept
    {
        if (!Base::_is_sso())
        {
            Allocator::template free<DataType>(Base::_data);
            Base::_reset_sso();
        }
    }
    inline SizeType grow(SizeType grow_size) noexcept
    {
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

        Base::set_size(new_size);
        return old_size;
    }
    inline void shrink() noexcept
    {
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
        if (Base::size())
        {
            memory::destruct(data(), Base::size());
            Base::set_size(0);
        }
    }

    // getter
    inline DataType*       data() noexcept { return reinterpret_cast<DataType*>(Base::_raw_data()); }
    inline const DataType* data() const noexcept { return reinterpret_cast<DataType*>(Base::_raw_data()); }
};

// COW string memory
struct COWStringMemory {
};

// serde string memory
struct SerdeStringMemory {
};

} // namespace skr::container