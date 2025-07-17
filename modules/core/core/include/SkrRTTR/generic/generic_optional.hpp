#pragma once
#include <SkrRTTR/generic/generic_base.hpp>
#include <SkrContainers/optional.hpp>

namespace skr
{
struct SKR_CORE_API GenericOptional final : IGenericBase {
    SKR_RC_IMPL(override final)

    // ctor & dtor
    GenericOptional(RC<IGenericBase> inner);
    ~GenericOptional();

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

    // has value
    bool  has_value(const void* memory) const;
    bool& has_value(void* memory) const;

    // value pointer
    void*       value_ptr(void* memory) const;
    const void* value_ptr(const void* memory) const;

    // reset
    void reset(void* memory) const;

    // assign
    void assign_value(void* dst, const void* v) const;
    void assign_value_move(void* dst, void* v) const;
    void assign_default(void* dst);

private:
    RC<IGenericBase> _inner            = nullptr;
    MemoryTraitsData _inner_mem_traits = {};
    uint64_t         _inner_size       = 0;
    uint64_t         _inner_alignment  = 0;
};
} // namespace skr