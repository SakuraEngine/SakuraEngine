#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/fwd_container.hpp"
#include "SkrBase/containers/sparse_array/sparse_array_def.hpp"
#include "SkrBase/misc/debug.h"
#include "SkrBase/containers/misc/placeholder.hpp"
#include "SkrBase/containers/allocator/allocator.hpp"

namespace skr::container
{
template <typename T, typename TBitBlock, typename TS, typename Allocator>
struct SparseArrayMemory : public Allocator {
    using SizeType           = TS;
    using BitBlockType       = TBitBlock;
    using DataType           = SparseArrayData<T, SizeType>;
    using AllocatorCtorParam = typename Allocator::CtorParam;

    // ctor & dtor
    SparseArrayMemory(AllocatorCtorParam param) noexcept;
    ~SparseArrayMemory() noexcept;

    // copy & move
    SparseArrayMemory(const SparseArrayMemory& other, AllocatorCtorParam param) noexcept;
    SparseArrayMemory(SparseArrayMemory&& other) noexcept;

    // assign & move assign
    SparseArrayMemory& operator=(const SparseArrayMemory& rhs) noexcept;
    SparseArrayMemory& operator=(SparseArrayMemory&& rhs) noexcept;

    // memory operations
    void realloc(SizeType new_capacity) noexcept;
    void free() noexcept;
    void grow(SizeType new_size) noexcept;
    void shrink() noexcept;

    // getter
    T*                  data() noexcept;
    const T*            data() const noexcept;
    BitBlockType*       bit_array() noexcept;
    const BitBlockType* bit_array() const noexcept;
    SizeType            size() const noexcept;
    SizeType            capacity() const noexcept;
    SizeType            bit_array_size() const noexcept;

    // setter
    void set_size(SizeType new_size) noexcept;

private:
    DataType*     _data;
    BitBlockType* _bit_array;
    SizeType      _size;
    SizeType      _capacity;
    SizeType      _bit_array_size;
};
} // namespace skr::container