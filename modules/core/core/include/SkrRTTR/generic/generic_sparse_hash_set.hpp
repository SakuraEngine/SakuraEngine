#pragma once
#include <SkrRTTR/generic/generic_base.hpp>
#include <SkrRTTR/generic/generic_sparase_hash_base.hpp>
#include <SkrContainers/set.hpp>

namespace skr
{
using GenericSparseHashSetDataRef  = GenericSparseHashSetDataRef;
using CGenericSparseHashSetDataRef = CGenericSparseHashSetDataRef;

struct SKR_CORE_API GenericSparseHashSet final : GenericSparseHashBase {
    using Super    = GenericSparseHashBase;
    using PredType = FunctionRef<bool(const void*)>;

    // ctor & dtor
    GenericSparseHashSet(RC<IGenericBase> inner);
    ~GenericSparseHashSet();

    //===> IGenericBase API
    // get type info
    EGenericKind kind() const override;
    GUID         id() const override;

    // get utils
    MemoryTraitsData memory_traits_data() const override;

    // basic info
    uint64_t size() const override;
    uint64_t alignment() const override;

    // operations, used for generic container algorithms
    bool   support(EGenericFeature feature) const override;
    void   default_ctor(void* dst, uint64_t count = 1) const override;
    void   dtor(void* dst, uint64_t count = 1) const override;
    void   copy(void* dst, const void* src, uint64_t count = 1) const override;
    void   move(void* dst, void* src, uint64_t count = 1) const override;
    void   assign(void* dst, const void* src, uint64_t count = 1) const override;
    void   move_assign(void* dst, void* src, uint64_t count = 1) const override;
    bool   equal(const void* lhs, const void* rhs, uint64_t count = 1) const override;
    size_t hash(const void* src) const override;
    void   swap(void* dst, void* src, uint64_t count = 1) const override;
    //===> IGenericBase API

    // getter
    bool                            is_valid() const;
    RC<IGenericBase>                inner() const;
    RC<GenericSparseHashSetStorage> inner_storage() const;

    // sparse hash set getter
    uint64_t        size(const void* dst) const;
    uint64_t        capacity(const void* dst) const;
    uint64_t        slack(const void* dst) const;
    uint64_t        sparse_size(const void* dst) const;
    uint64_t        hole_size(const void* dst) const;
    uint64_t        bit_size(const void* dst) const;
    uint64_t        freelist_head(const void* dst) const;
    bool            is_compact(const void* dst) const;
    bool            is_empty(const void* dst) const;
    void*           storage(void* dst) const;
    const void*     storage(const void* dst) const;
    uint64_t*       bit_data(void* dst) const;
    const uint64_t* bit_data(const void* dst) const;
    uint64_t*       bucket(void* dst) const;
    const uint64_t* bucket(const void* dst) const;
    uint64_t        bucket_size(const void* dst) const;
    uint64_t&       bucket_size(void* dst) const;
    uint64_t        bucket_mask(const void* dst) const;
    uint64_t&       bucket_mask(void* dst) const;

    // validator
    using Super::has_data;
    using Super::is_hole;
    using Super::is_valid_index;

    // memory op
    using Super::clear;
    using Super::release;
    using Super::reserve;
    using Super::shrink;
    using Super::compact;
    using Super::compact_stable;
    using Super::compact_top;

    // rehash
    using Super::rehash;
    using Super::rehash_if_need;

    // add
    GenericSparseHashSetDataRef add(void* dst, const void* v);
    GenericSparseHashSetDataRef add_move(void* dst, void* v);

    // append
    void append(void* dst, const void* src);

    // remove
    using Super::remove_at;
    using Super::remove_at_unsafe;
    bool remove(void* dst, const void* v);

    // remove if
    using Super::remove_if;
    using Super::remove_last_if;
    using Super::remove_all_if;

    // find
    GenericSparseHashSetDataRef  find(void* dst, const void* v) const;
    CGenericSparseHashSetDataRef find(const void* dst, const void* v) const;

    // find if
    GenericSparseHashSetDataRef  find_if(void* dst, PredType pred) const;
    GenericSparseHashSetDataRef  find_last_if(void* dst, PredType pred) const;
    CGenericSparseHashSetDataRef find_if(const void* dst, PredType pred) const;
    CGenericSparseHashSetDataRef find_last_if(const void* dst, PredType pred) const;

    // contains
    bool contains(const void* dst, const void* v) const;

    // contains if
    bool contains_if(const void* dst, PredType pred) const;

    // visitor
    const void* at(const void* dst, uint64_t idx) const;
    const void* at_last(const void* dst, uint64_t idx) const;

    // set ops
    bool is_sub_set_of(const void* dst, const void* rhs) const;
};
} // namespace skr