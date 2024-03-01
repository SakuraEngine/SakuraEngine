#pragma once
#include "SkrBase/containers/sparse_vector/sparse_vector_iterator.hpp"

namespace skr::container
{
template <typename Container, bool kConst>
struct SparseHashMapCursor;

template <typename Container, bool kConst>
struct SparseHashMapIter : public CursorIter<SparseHashMapCursor<Container, kConst>, false> {
    using Super = CursorIter<SparseHashMapCursor<Container, kConst>, false>;
    using Super::Super;

    using Super::key;
    using Super::value;

    using Super::hash;

    inline void erase_and_move_next()
    {
        Super::cursor().erase_and_move_next();
    }
};
template <typename Container, bool kConst>
struct SparseHashMapIterInv : public CursorIter<SparseHashMapCursor<Container, kConst>, true> {
    using Super = CursorIter<SparseHashMapCursor<Container, kConst>, true>;
    using Super::Super;

    using Super::key;
    using Super::value;

    using Super::hash;

    inline void erase_and_move_next()
    {
        Super::cursor().erase_and_move_prev();
    }
};

template <typename Container, bool kConst>
struct SparseHashMapCursor : protected SparseVectorCursor<Container, kConst> {
    using Super         = SparseVectorCursor<Container, kConst>;
    using ContainerType = std::conditional_t<kConst, const Container, Container>;
    using SizeType      = typename ContainerType::SizeType;
    using DataType      = std::conditional_t<kConst, const typename ContainerType::SetDataType, typename ContainerType::SetDataType>;
    using HashType      = typename ContainerType::HashType;
    using KeyType       = std::conditional_t<kConst, const typename ContainerType::MapKeyType, typename ContainerType::MapKeyType>;
    using ValueType     = std::conditional_t<kConst, const typename ContainerType::MapValueType, typename ContainerType::MapValueType>;

    // ctor & copy & move & assign & move assign
    inline SparseHashMapCursor(ContainerType* container, SizeType index)
        : Super(container, index)
    {
    }
    inline SparseHashMapCursor(ContainerType* container)
        : Super(container)
    {
    }
    inline SparseHashMapCursor(const SparseHashMapCursor& rhs)            = default;
    inline SparseHashMapCursor(SparseHashMapCursor&& rhs)                 = default;
    inline SparseHashMapCursor& operator=(const SparseHashMapCursor& rhs) = default;
    inline SparseHashMapCursor& operator=(SparseHashMapCursor&& rhs)      = default;

    // factory
    inline static SparseHashMapCursor Begin(ContainerType* container)
    {
        SparseHashMapCursor cursor{ container };
        cursor.reset_to_begin();
        return cursor;
    }
    inline static SparseHashMapCursor BeginOverflow(ContainerType* container)
    {
        SparseHashMapCursor cursor{ container };
        cursor._reset_to_begin_overflow();
        return cursor;
    }
    inline static SparseHashMapCursor End(ContainerType* container)
    {
        SparseHashMapCursor cursor{ container };
        cursor.reset_to_end();
        return cursor;
    }
    inline static SparseHashMapCursor EndOverflow(ContainerType* container)
    {
        SparseHashMapCursor cursor{ container };
        cursor._reset_to_end_overflow();
        return cursor;
    }

    // getter
    inline DataType&  ref() const { return Super::ref()._sparse_hash_set_data; }
    inline DataType*  ptr() const { return &Super::ref()._sparse_hash_set_data; }
    inline KeyType&   key() const { return ref().key; }
    inline ValueType& value() const { return ref().value; }
    inline HashType   hash() const { return Super::ref()._sparse_hash_set_hash; }
    inline SizeType   index() const { return Super::index(); }

    // move & validator
    using Super::move_next;
    using Super::move_prev;
    using Super::reset_to_begin;
    using Super::reset_to_end;

    // erase
    using Super::erase_and_move_next;
    using Super::erase_and_move_prev;

    // reach & validate
    using Super::reach_begin;
    using Super::reach_end;
    using Super::is_valid;

    // compare
    inline bool operator==(const SparseHashMapCursor& rhs) const { return Super::operator==(rhs); }
    inline bool operator!=(const SparseHashMapCursor& rhs) const { return Super::operator!=(rhs); }

    // convert
    inline SparseHashMapIter<ContainerType, kConst>    as_iter() const { return { *this }; }
    inline SparseHashMapIterInv<ContainerType, kConst> as_iter_inv() const { return { *this }; }
    inline CursorRange<SparseHashMapCursor, false>     as_range() const { return { *this }; }
    inline CursorRange<SparseHashMapCursor, true>      as_range_inv() const { return { *this }; }
};

} // namespace skr::container