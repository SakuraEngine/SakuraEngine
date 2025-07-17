#pragma once
#include <SkrRTTR/generic/generic_base.hpp>
#include <SkrContainers/vector.hpp>

namespace skr
{
// data ref
struct GenericVectorDataRef {
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
    inline void* offset(uint64_t index, uint64_t item_size) const
    {
        SKR_ASSERT(is_valid());
        SKR_ASSERT(index < npos_of<uint64_t>);
        return ::skr::memory::offset_item(ptr, item_size, index);
    }
};
struct CGenericVectorDataRef {
    const void* ptr   = nullptr;
    uint64_t    index = 0;

    inline CGenericVectorDataRef(const GenericVectorDataRef& ref)
        : ptr(ref.ptr)
        , index(ref.index)
    {
    }
    inline operator GenericVectorDataRef() const
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
    inline const void* offset(uint64_t index, uint64_t item_size) const
    {
        SKR_ASSERT(is_valid());
        SKR_ASSERT(index < npos_of<uint64_t>);
        return ::skr::memory::offset_item(ptr, item_size, index);
    }
};

// generic vector
struct SKR_CORE_API GenericVector final : IGenericBase {
    SKR_RC_IMPL(override final)
    using PredType = FunctionRef<bool(const void*)>;

    // ctor & dtor
    GenericVector(RC<IGenericBase> inner);
    ~GenericVector();

    //===> IGenericBase API
    // get type info
    EGenericKind kind() const override final;
    GUID         id() const override final;

    // get utils
    MemoryTraitsData memory_traits_data() const override final;

    // basic info
    uint64_t size() const override final;
    uint64_t alignment() const override final;

    // operations, used for generic container algorithms
    bool   support(EGenericFeature feature) const override final;
    void   default_ctor(void* dst, uint64_t count = 1) const override final;
    void   dtor(void* dst, uint64_t count = 1) const override final;
    void   copy(void* dst, const void* src, uint64_t count = 1) const override final;
    void   move(void* dst, void* src, uint64_t count = 1) const override final;
    void   assign(void* dst, const void* src, uint64_t count = 1) const override final;
    void   move_assign(void* dst, void* src, uint64_t count = 1) const override final;
    bool   equal(const void* lhs, const void* rhs, uint64_t count = 1) const override final;
    size_t hash(const void* src) const override final;
    void   swap(void* dst, void* src, uint64_t count = 1) const override final;
    //===> IGenericBase API

    // getter
    bool             is_valid() const;
    RC<IGenericBase> inner() const;

    // vector getter
    uint64_t    size(const void* dst) const;
    uint64_t    capacity(const void* dst) const;
    uint64_t    slack(const void* dst) const;
    bool        is_empty(const void* dst) const;
    const void* data(const void* dst) const;
    void*       data(void* dst) const;

    // validate
    bool is_valid_index(const void* dst, uint64_t idx) const;

    // memory op
    void clear(void* dst) const;
    void release(void* dst, uint64_t reserve_capacity = 0) const;
    void reserve(void* dst, uint64_t expect_capacity) const;
    void shrink(void* dst) const;
    void resize(void* dst, uint64_t expect_size, void* new_value) const;
    void resize_unsafe(void* dst, uint64_t expect_size) const;
    void resize_default(void* dst, uint64_t expect_size) const;
    void resize_zeroed(void* dst, uint64_t expect_size) const;

    // add
    GenericVectorDataRef add(void* dst, const void* v, uint64_t n = 1) const;
    GenericVectorDataRef add_move(void* dst, void* v) const;
    GenericVectorDataRef add_unsafe(void* dst, uint64_t n = 1) const;
    GenericVectorDataRef add_default(void* dst, uint64_t n = 1) const;
    GenericVectorDataRef add_zeroed(void* dst, uint64_t n = 1) const;
    GenericVectorDataRef add_unique(void* dst, const void* v) const;
    GenericVectorDataRef add_unique_move(void* dst, void* v) const;

    // add at
    void add_at(void* dst, uint64_t idx, const void* v, uint64_t n = 1) const;
    void add_at_move(void* dst, uint64_t idx, void* v) const;
    void add_at_unsafe(void* dst, uint64_t idx, uint64_t n = 1) const;
    void add_at_default(void* dst, uint64_t idx, uint64_t n = 1) const;
    void add_at_zeroed(void* dst, uint64_t idx, uint64_t n = 1) const;

    // append
    GenericVectorDataRef append(void* dst, const void* other) const;

    // append at
    void append_at(void* dst, uint64_t idx, const void* other) const;

    // remove
    void     remove_at(void* dst, uint64_t idx, uint64_t n = 1) const;
    void     remove_at_swap(void* dst, uint64_t idx, uint64_t n = 1) const;
    bool     remove(void* dst, const void* v) const;
    bool     remove_swap(void* dst, const void* v) const;
    bool     remove_last(void* dst, const void* v) const;
    bool     remove_last_swap(void* dst, const void* v) const;
    uint64_t remove_all(void* dst, const void* v) const;
    uint64_t remove_all_swap(void* dst, const void* v) const;

    // remove if
    bool     remove_if(void* dst, PredType pred) const;
    bool     remove_swap_if(void* dst, PredType pred) const;
    bool     remove_last_if(void* dst, PredType pred) const;
    bool     remove_last_swap_if(void* dst, PredType pred) const;
    uint64_t remove_all_if(void* dst, PredType pred) const;
    uint64_t remove_all_swap_if(void* dst, PredType pred) const;

    // access
    void*       at(void* dst, uint64_t idx) const;
    void*       at_last(void* dst, uint64_t idx) const;
    const void* at(const void* dst, uint64_t idx) const;
    const void* at_last(const void* dst, uint64_t idx) const;

    // find
    GenericVectorDataRef  find(void* dst, const void* v) const;
    GenericVectorDataRef  find_last(void* dst, const void* v) const;
    CGenericVectorDataRef find(const void* dst, const void* v) const;
    CGenericVectorDataRef find_last(const void* dst, const void* v) const;

    // find if
    GenericVectorDataRef  find_if(void* dst, PredType pred) const;
    GenericVectorDataRef  find_last_if(void* dst, PredType pred) const;
    CGenericVectorDataRef find_if(const void* dst, PredType pred) const;
    CGenericVectorDataRef find_last_if(const void* dst, PredType pred) const;

    // contains & count
    bool     contains(const void* dst, const void* v) const;
    uint64_t count(const void* dst, const void* v) const;

    // contains if & count if
    bool     contains_if(const void* dst, PredType pred) const;
    uint64_t count_if(const void* dst, PredType pred) const;

    // sort
    // TODO. 需要比较 op，Swapper
    // void sort_less(void* dst) const;
    // void sort_greater(void* dst) const;
    // void sort_stable_less(void* dst) const;
    // void sort_stable_greater(void* dst) const;

private:
    // helper
    void     _realloc(void* dst, uint64_t new_capacity) const;
    void     _free(void* dst) const;
    uint64_t _grow(void* dst, uint64_t grow_size) const;

private:
    RC<IGenericBase> _inner            = nullptr;
    MemoryTraitsData _inner_mem_traits = {};
    uint64_t         _inner_size       = 0;
    uint64_t         _inner_alignment  = 0;
};

// TODO. 一套 Cursor 和迭代器用于元素遍历
// TODO. ItemProxy 用于代理元素

} // namespace skr