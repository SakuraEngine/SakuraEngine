#pragma once
#include <SkrRTTR/generic/generic_base.hpp>
#include <SkrContainers/sparse_vector.hpp>

namespace skr
{
// data ref
struct GenericSparseVectorDataRef {
    void*    ptr   = nullptr;
    uint64_t index = npos_of<uint64_t>;

    inline bool is_valid() const
    {
        return ptr != nullptr && index != npos_of<uint64_t>;
    }
    inline operator bool()
    {
        return is_valid();
    }
};
struct CGenericSparseVectorDataRef {
    const void* ptr   = nullptr;
    uint64_t    index = 0;

    inline CGenericSparseVectorDataRef(const GenericSparseVectorDataRef& ref)
        : ptr(ref.ptr)
        , index(ref.index)
    {
    }
    inline operator GenericSparseVectorDataRef() const
    {
        return { const_cast<void*>(ptr), index };
    }

    inline bool is_valid() const
    {
        return ptr != nullptr && index != npos_of<uint64_t>;
    }
    inline operator bool()
    {
        return is_valid();
    }
};

// generic vector
struct SKR_CORE_API GenericSparseVector : IGenericBase {
    SKR_RC_IMPL(override final)
    using BitAlgo        = algo::BitAlgo<uint64_t>;
    using TrueBitCursor  = container::BitCursor<uint64_t, uint64_t, false>;
    using CTrueBitCursor = container::BitCursor<uint64_t, uint64_t, true>;
    using PredType       = FunctionRef<bool(const void*)>;

    inline static constexpr uint64_t npos = npos_of<uint64_t>;

    // ctor & dtor
    GenericSparseVector(RC<IGenericBase> inner);
    ~GenericSparseVector();

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

    // sparse vector getter
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

    // validate
    bool has_data(const void* dst, uint64_t idx) const;
    bool is_hole(const void* dst, uint64_t idx) const;
    bool is_valid_index(const void* dst, uint64_t idx) const;

    // memory op
    void clear(void* dst) const;
    void release(void* dst, uint64_t reserve_capacity = 0) const;
    void reserve(void* dst, uint64_t expect_capacity) const;
    void shrink(void* dst) const;
    bool compact(void* dst) const;
    bool compact_stable(void* dst) const;
    bool compact_top(void* dst) const;

    // add
    GenericSparseVectorDataRef add(void* dst, const void* v) const;
    GenericSparseVectorDataRef add_move(void* dst, void* v) const;
    GenericSparseVectorDataRef add_unsafe(void* dst) const;
    GenericSparseVectorDataRef add_default(void* dst) const;
    GenericSparseVectorDataRef add_zeroed(void* dst) const;

    // add at
    void add_at(void* dst, uint64_t idx, const void* v) const;
    void add_at_move(void* dst, uint64_t idx, void* v) const;
    void add_at_unsafe(void* dst, uint64_t idx) const;
    void add_at_default(void* dst, uint64_t idx) const;
    void add_at_zeroed(void* dst, uint64_t idx) const;

    // append
    void append(void* dst, const void* other) const;

    // remove
    void     remove_at(void* dst, uint64_t idx, uint64_t n = 1) const;
    void     remove_at_unsafe(void* dst, uint64_t idx, uint64_t n = 1) const;
    bool     remove(void* dst, const void* v) const;
    bool     remove_last(void* dst, const void* v) const;
    uint64_t remove_all(void* dst, const void* v) const;

    // remove if
    bool     remove_if(void* dst, PredType pred) const;
    bool     remove_last_if(void* dst, PredType pred) const;
    uint64_t remove_all_if(void* dst, PredType pred) const;

    // access
    void*       at(void* dst, uint64_t idx) const;
    void*       at_last(void* dst, uint64_t idx) const;
    const void* at(const void* dst, uint64_t idx) const;
    const void* at_last(const void* dst, uint64_t idx) const;

    // find
    GenericSparseVectorDataRef  find(void* dst, const void* v) const;
    GenericSparseVectorDataRef  find_last(void* dst, const void* v) const;
    CGenericSparseVectorDataRef find(const void* dst, const void* v) const;
    CGenericSparseVectorDataRef find_last(const void* dst, const void* v) const;

    // find if
    GenericSparseVectorDataRef  find_if(void* dst, PredType pred) const;
    GenericSparseVectorDataRef  find_last_if(void* dst, PredType pred) const;
    CGenericSparseVectorDataRef find_if(const void* dst, PredType pred) const;
    CGenericSparseVectorDataRef find_last_if(const void* dst, PredType pred) const;

    // contains & count
    bool     contains(const void* dst, const void* v) const;
    uint64_t count(const void* dst, const void* v) const;

    // contains if & count if
    bool     contains_if(const void* dst, PredType pred) const;
    uint64_t count_if(const void* dst, PredType pred) const;

private:
    // helper
    void     _realloc(void* dst, uint64_t new_capacity) const;
    void     _free(void* dst) const;
    uint64_t _grow(void* dst, uint64_t grow_size) const;
    void     _copy_sparse_vector_data(void* dst, const void* src, const void* src_bit_data, uint64_t size) const;
    void     _move_sparse_vector_data(void* dst, void* src, const void* src_bit_data, uint64_t size) const;
    void     _copy_sparse_vector_bit_data(void* dst, const void* src, uint64_t size) const;
    void     _move_sparse_vector_bit_data(void* dst, void* src, uint64_t size) const;
    void     _destruct_sparse_vector_data(void* dst, const void* bit_data, uint64_t size) const;
    void     _break_freelist_at(void* dst, uint64_t idx) const;

private:
    RC<IGenericBase> _inner            = nullptr;
    MemoryTraitsData _inner_mem_traits = {};
    uint64_t         _inner_size       = 0;
    uint64_t         _inner_alignment  = 0;
};

} // namespace skr