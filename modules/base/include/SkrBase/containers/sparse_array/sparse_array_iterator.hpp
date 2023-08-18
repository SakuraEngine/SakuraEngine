#pragma once
#include "SkrBase/config.h"
#include "sparse_array_def.hpp"
#include "SkrBase/containers/bit_array/bit_iterator.hpp"

// SparseArray iterator
namespace skr
{
template <typename T, typename TBitBlock, typename TS, bool Const>
struct SparseArrayIt {
    using DataType  = std::conditional_t<Const, const SparseArrayData<T, TS>, SparseArrayData<T, TS>>;
    using ValueType = std::conditional_t<Const, const T, T>;
    using BitItType = TrueBitIt<TBitBlock, TS, true>;

    SKR_INLINE explicit SparseArrayIt(DataType* array, TS array_size, const TBitBlock* bit_array, TS start = 0)
        : _array(array)
        , _bit_it(bit_array, array_size, start)
    {
    }

    // impl cpp iterator
    SKR_INLINE SparseArrayIt& operator++()
    {
        ++_bit_it;
        return *this;
    }
    SKR_INLINE bool operator==(const SparseArrayIt& rhs) const { return _bit_it == rhs._bit_it && _array == rhs._array; }
    SKR_INLINE bool operator!=(const SparseArrayIt& rhs) const { return !(*this == rhs); }
    SKR_INLINE operator bool() const { return (bool)_bit_it; }
    SKR_INLINE bool       operator!() const { return !(bool)*this; }
    SKR_INLINE ValueType& operator*() const { return _array[index()]._sparse_array_data; }
    SKR_INLINE ValueType* operator->() const { return &_array[index()]._sparse_array_data; }

    // other data
    SKR_INLINE ValueType* data() const { return &_array[index()]._sparse_array_data; }
    SKR_INLINE TS         index() const { return _bit_it.index(); }

private:
    DataType* _array;
    BitItType _bit_it;
};
} // namespace skr