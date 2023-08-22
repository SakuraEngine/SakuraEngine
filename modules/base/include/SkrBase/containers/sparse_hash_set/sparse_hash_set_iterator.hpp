#pragma once
#include "SkrBase/config.h"
#include "SkrBase/tools/integer_tools.hpp"
#include "sparse_hash_set_def.hpp"
#include "SkrBase/containers/sparse_array/sparse_array_iterator.hpp"

// SparseHashSet iterator
namespace skr
{
template <typename T, typename TBitBlock, typename TS, typename TH, bool Const>
struct SparseHashSetIt {
    using DataType       = SparseHashSetData<T, TS, TH>;
    using SparseDataType = std::conditional_t<Const, const SparseArrayData<DataType, TS>, SparseArrayData<DataType, TS>>;
    using ValueType      = std::conditional_t<Const, const T, T>;
    using BitItType      = TrueBitIt<TBitBlock, TS, true>;

    SKR_INLINE explicit SparseHashSetIt(SparseDataType* array, TS array_size, const TBitBlock* bit_array, TS start = 0)
        : _array(array)
        , _bit_it(bit_array, array_size, start)
    {
    }

    // impl cpp iterator
    SKR_INLINE SparseHashSetIt& operator++()
    {
        ++_bit_it;
        return *this;
    }
    SKR_INLINE bool operator==(const SparseHashSetIt& rhs) const { return _bit_it == rhs._bit_it && _array == rhs._array; }
    SKR_INLINE bool operator!=(const SparseHashSetIt& rhs) const { return !(*this == rhs); }
    SKR_INLINE operator bool() const { return (bool)_bit_it; }
    SKR_INLINE bool       operator!() const { return !(bool)*this; }
    SKR_INLINE ValueType& operator*() const { return _array[index()]._sparse_array_data._sparse_hash_set_data; }
    SKR_INLINE ValueType* operator->() const { return &_array[index()]._sparse_array_data._sparse_hash_set_data; }

    // other data
    SKR_INLINE TS index() const { return _bit_it.index(); }
    SKR_INLINE TH hash() const { return _array[index()]._sparse_array_data._sparse_hash_set_hash; }

private:
    SparseDataType* _array;
    BitItType       _bit_it;
};
} // namespace skr
