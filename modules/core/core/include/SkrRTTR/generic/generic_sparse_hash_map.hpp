#pragma once
#include <SkrRTTR/generic/generic_base.hpp>
#include <SkrRTTR/generic/generic_sparase_hash_base.hpp>
#include <SkrContainers/map.hpp>

// generic kv pair
namespace skr
{
using GenericSparseHashMapDataRef  = GenericSparseHashSetDataRef;
using CGenericSparseHashMapDataRef = CGenericSparseHashSetDataRef;

struct SKR_CORE_API GenericKVPair final : IGenericBase {
    SKR_RC_IMPL(override final)

    // ctor & dtor
    GenericKVPair(RC<IGenericBase> inner_key, RC<IGenericBase> inner_value);
    ~GenericKVPair() override;

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
    bool             is_valid() const;
    RC<IGenericBase> inner_key() const;
    RC<IGenericBase> inner_value() const;

    // get key & value
    void*       key(void* dst) const;
    void*       value(void* dst) const;
    const void* key(const void* dst) const;
    const void* value(const void* dst) const;

private:
    RC<IGenericBase> _inner_key            = nullptr;
    MemoryTraitsData _inner_key_mem_traits = {};
    uint64_t         _inner_key_size       = 0;
    uint64_t         _inner_key_alignment  = 0;

    RC<IGenericBase> _inner_value            = nullptr;
    MemoryTraitsData _inner_value_mem_traits = {};
    uint64_t         _inner_value_size       = 0;
    uint64_t         _inner_value_alignment  = 0;
};

struct SKR_CORE_API GenericSparseHashMap final : GenericSparseHashBase {
    using Super    = GenericSparseHashBase;
    using PredType = FunctionRef<bool(const void*)>;

    // ctor & dtor
    GenericSparseHashMap(RC<IGenericBase> inner_key, RC<IGenericBase> inner_value);
    ~GenericSparseHashMap();

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
    bool              is_valid() const;
    RC<IGenericBase>  inner_key() const;
    RC<IGenericBase>  inner_value() const;
    RC<GenericKVPair> inner_kv_pair() const;

    // sparse hash map getter
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
    GenericSparseHashMapDataRef add(void* dst, const void* key, const void* value);
    GenericSparseHashMapDataRef add_move(void* dst, void* key, void* value);
    GenericSparseHashMapDataRef add_ex_unsafe(void* dst, size_t hash, PredType pred);

    // try add (key only add)
    GenericSparseHashMapDataRef try_add_unsafe(void* dst, const void* key);
    GenericSparseHashMapDataRef try_add_move_unsafe(void* dst, void* key);

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
    GenericSparseHashMapDataRef  find(void* dst, const void* key) const;
    CGenericSparseHashMapDataRef find(const void* dst, const void* key) const;

    // find if
    GenericSparseHashMapDataRef  find_if(void* dst, PredType pred) const;
    GenericSparseHashMapDataRef  find_last_if(void* dst, PredType pred) const;
    CGenericSparseHashMapDataRef find_if(const void* dst, PredType pred) const;
    CGenericSparseHashMapDataRef find_last_if(const void* dst, PredType pred) const;

    // contains
    bool contains(const void* dst, const void* key) const;

    // contains if
    bool contains_if(const void* dst, PredType pred) const;

    // visitor
    const void* at(const void* dst, uint64_t idx) const;
    const void* at_last(const void* dst, uint64_t idx) const;

private:
    RC<GenericKVPair> _inner_kv_pair = nullptr;
};

} // namespace skr