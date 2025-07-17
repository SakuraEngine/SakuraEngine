#pragma once
#include <SkrRTTR/generic/generic_base.hpp>
#include <SkrRTTR/generic/generic_sparse_vector.hpp>

namespace skr
{
struct SKR_CORE_API GenericSparseHashSetStorage final : IGenericBase {
    SKR_RC_IMPL(override final)

    inline static constexpr uint64_t npos = skr::npos_of<uint64_t>;

    // ctor & dtor
    GenericSparseHashSetStorage(RC<IGenericBase> inner);
    ~GenericSparseHashSetStorage();

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
    RC<IGenericBase> inner() const;

    // item getter
    void*       item_data(void* dst) const;
    const void* item_data(const void* dst) const;
    size_t      item_hash(const void* dst) const;
    size_t&     item_hash(void* dst) const;
    uint64_t    next(const void* dst) const;
    uint64_t&   next(void* dst) const;

private:
    RC<IGenericBase> _inner            = nullptr;
    MemoryTraitsData _inner_mem_traits = {};
    uint64_t         _inner_size       = 0;
    uint64_t         _inner_alignment  = 0;
};

// data def
struct GenericSparseHashSetDataRef {
    void*    ptr           = nullptr;           // data pointer
    uint64_t index         = npos_of<uint64_t>; // index in the sparse hash set
    size_t   hash          = 0;                 // hash value
    bool     already_exist = false;             // if the data already exist in the sparse hash set

    inline bool is_valid() const { return ptr != nullptr && index != skr::npos_of<uint64_t>; }
    inline      operator bool() const { return is_valid(); }
};
struct CGenericSparseHashSetDataRef {
    const void* ptr           = nullptr;           // data pointer
    uint64_t    index         = npos_of<uint64_t>; // index in the sparse hash set
    size_t      hash          = 0;                 // hash value
    bool        already_exist = false;             // if the data already exist in the sparse hash set

    inline CGenericSparseHashSetDataRef(const GenericSparseHashSetDataRef& ref)
        : ptr(ref.ptr)
        , index(ref.index)
        , hash(ref.hash)
        , already_exist(ref.already_exist)
    {
    }

    inline operator GenericSparseHashSetDataRef() const
    {
        return {
            const_cast<void*>(ptr),
            index,
            hash,
            already_exist
        };
    }

    inline bool is_valid() const { return ptr != nullptr && index != skr::npos_of<uint64_t>; }
    inline      operator bool() const { return is_valid(); }
};

// GenericSparseHashSetBase
struct SKR_CORE_API GenericSparseHashBase : GenericSparseVector {
    using Super    = GenericSparseVector;
    using PredType = FunctionRef<bool(const void*)>;

    // ctor & dtor
    GenericSparseHashBase(RC<IGenericBase> inner);
    ~GenericSparseHashBase();

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

protected:
    // getter
    bool                            is_valid() const;
    RC<IGenericBase>                inner() const;
    RC<GenericSparseHashSetStorage> inner_storage() const;

    // sparse hash set getter
    using Super::size;
    using Super::capacity;
    using Super::slack;
    using Super::sparse_size;
    using Super::hole_size;
    using Super::bit_size;
    using Super::freelist_head;
    using Super::is_compact;
    using Super::is_empty;
    using Super::storage;
    using Super::bit_data;
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
    void clear(void* dst) const;
    void release(void* dst, uint64_t capacity = 0) const;
    void reserve(void* dst, uint64_t capacity) const;
    void shrink(void* dst) const;
    bool compact(void* dst) const;
    bool compact_stable(void* dst) const;
    bool compact_top(void* dst) const;

    // rehash
    void rehash(void* dst) const;
    bool rehash_if_need(void* dst) const;

    // visitor
    const void* at(const void* dst, uint64_t idx) const;
    const void* at_last(const void* dst, uint64_t idx) const;

    // remove
    void remove_at(void* dst, uint64_t idx) const;
    void remove_at_unsafe(void* dst, uint64_t idx) const;

    // basic add/find/remove
    GenericSparseHashSetDataRef add_unsafe(void* dst, size_t hash) const;
    GenericSparseHashSetDataRef find(const void* dst, size_t hash, PredType pred) const;
    GenericSparseHashSetDataRef find_next(const void* dst, GenericSparseHashSetDataRef ref, PredType pred) const;
    bool                        remove(void* dst, size_t hash, PredType pred) const;
    uint64_t                    remove_all(void* dst, size_t hash, PredType pred) const;

    // find if
    GenericSparseHashSetDataRef find_if(void* dst, PredType pred) const;
    GenericSparseHashSetDataRef find_last_if(void* dst, PredType pred) const;

private:
    // helper
    void     _free_bucket(void* dst) const;
    bool     _resize_bucket(void* dst) const;
    void     _clean_bucket(void* dst) const;
    void     _build_bucket(void* dst) const;
    uint64_t _bucket_index(const void* dst, uint64_t hash) const;
    bool     _is_in_bucket(const void* dst, uint64_t index) const;
    void     _add_to_bucket(void* dst, void* item_data, uint64_t index) const;
    void     _remove_from_bucket(void* dst, uint64_t index) const;

protected:
    RC<IGenericBase> _inner            = nullptr;
    MemoryTraitsData _inner_mem_traits = {};
    uint64_t         _inner_size       = 0;
    uint64_t         _inner_alignment  = 0;

    RC<GenericSparseHashSetStorage> _inner_storage = nullptr;
};

} // namespace skr