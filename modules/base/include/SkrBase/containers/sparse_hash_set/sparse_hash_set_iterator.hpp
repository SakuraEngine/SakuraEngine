#pragma once
#include "SkrBase/containers/sparse_vector/sparse_vector_iterator.hpp"

namespace skr::container
{
template <typename Container, bool kConst>
struct SparseHashSetCursor;

template <typename Container, bool kConst>
struct SparseHashSetIter : public CursorIter<SparseHashSetCursor<Container, kConst>, false> {
    using Super = CursorIter<SparseHashSetCursor<Container, kConst>, false>;
    using Super::Super;

    inline void erase_and_move_next()
    {
        Super::cursor().erase_and_move_next();
    }
};

template <typename Container, bool kConst>
struct SparseHashSetIterInv : public CursorIter<SparseHashSetCursor<Container, kConst>, true> {
    using Super = CursorIter<SparseHashSetCursor<Container, kConst>, true>;
    using Super::Super;

    inline void erase_and_move_next()
    {
        Super::cursor().erase_and_move_prev();
    }
};

template <typename Container, bool kConst>
struct SparseHashSetCursor : protected SparseVectorCursor<Container, kConst> {
    using Super         = SparseVectorCursor<Container, kConst>;
    using ContainerType = std::conditional_t<kConst, const Container, Container>;
    using SizeType      = typename ContainerType::SizeType;
    using DataType      = std::conditional_t<kConst, const typename ContainerType::SetDataType, typename ContainerType::SetDataType>;
    using HashType      = typename ContainerType::HashType;

    // ctor & copy & move & assign & move assign
    inline SparseHashSetCursor(ContainerType* container, SizeType index)
        : Super(container, index)
    {
    }
    inline SparseHashSetCursor(ContainerType* container)
        : Super(container)
    {
    }
    inline SparseHashSetCursor(const SparseHashSetCursor& rhs)            = default;
    inline SparseHashSetCursor(SparseHashSetCursor&& rhs)                 = default;
    inline SparseHashSetCursor& operator=(const SparseHashSetCursor& rhs) = default;
    inline SparseHashSetCursor& operator=(SparseHashSetCursor&& rhs)      = default;

    // factory
    inline static SparseHashSetCursor Begin(ContainerType* container)
    {
        SparseHashSetCursor cursor{ container };
        cursor.reset_to_begin();
        return cursor;
    }
    inline static SparseHashSetCursor BeginOverflow(ContainerType* container)
    {
        SparseHashSetCursor cursor{ container };
        cursor._reset_to_begin_overflow();
        return cursor;
    }
    inline static SparseHashSetCursor End(ContainerType* container)
    {
        SparseHashSetCursor cursor{ container };
        cursor.reset_to_end();
        return cursor;
    }
    inline static SparseHashSetCursor EndOverflow(ContainerType* container)
    {
        SparseHashSetCursor cursor{ container };
        cursor._reset_to_end_overflow();
        return cursor;
    }

    // getter
    inline DataType& ref() const { return Super::ref()._sparse_hash_set_data; }
    inline DataType* ptr() const { return &Super::ref()._sparse_hash_set_data; }
    inline HashType  hash() const { return Super::ref()._sparse_hash_set_hash; }
    inline SizeType  index() const { return Super::index(); }

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
    inline bool operator==(const SparseHashSetCursor& rhs) const { return Super::operator==(rhs); }
    inline bool operator!=(const SparseHashSetCursor& rhs) const { return Super::operator!=(rhs); }

    // convert
    inline SparseHashSetIter<ContainerType, kConst>    as_iter() const { return { *this }; }
    inline SparseHashSetIterInv<ContainerType, kConst> as_iter_inv() const { return { *this }; }
    inline CursorRange<SparseHashSetCursor, false>     as_range() const { return { *this }; }
    inline CursorRange<SparseHashSetCursor, true>      as_range_inv() const { return { *this }; }
};
} // namespace skr::container
