#pragma once
#include <SkrRTTR/type.hpp>
#include <SkrCore/log.hpp>
#include <SkrContainers/optional.hpp>
#include <SkrContainers/vector.hpp>
#include <SkrCore/memory/rc.hpp>
#include <SkrContainers/not_null.hpp>

namespace skr
{
// generic interface
enum class EGenericKind
{
    Generic,
    Type,
};
enum class EGenericFeature
{
    DefaultCtor,
    Dtor,
    Copy,
    Move,
    Assign,
    MoveAssign,
    Equal,
    Hash,
    Swap,
};
struct SKR_CORE_API IGenericBase {
    SKR_RC_INTEFACE()
    virtual ~IGenericBase();

    // get type info
    virtual EGenericKind kind() const = 0;
    virtual GUID         id() const   = 0; // type or generic id, depends on kind

    // get utils
    virtual MemoryTraitsData memory_traits_data() const = 0;

    // basic info
    virtual uint64_t size() const      = 0;
    virtual uint64_t alignment() const = 0;

    // operations, used for generic container algorithms
    virtual bool   support(EGenericFeature feature) const                            = 0;
    virtual void   default_ctor(void* dst, uint64_t count = 1) const                 = 0;
    virtual void   dtor(void* dst, uint64_t count = 1) const                         = 0;
    virtual void   copy(void* dst, const void* src, uint64_t count = 1) const        = 0;
    virtual void   move(void* dst, void* src, uint64_t count = 1) const              = 0;
    virtual void   assign(void* dst, const void* src, uint64_t count = 1) const      = 0;
    virtual void   move_assign(void* dst, void* src, uint64_t count = 1) const       = 0;
    virtual bool   equal(const void* lhs, const void* rhs, uint64_t count = 1) const = 0;
    virtual size_t hash(const void* src) const                                       = 0;
    virtual void   swap(void* dst, void* src, uint64_t count = 1) const              = 0;
};

enum class EGenericTypeFlag : uint8_t
{
    None      = 0,
    Const     = 1 << 0, // const pointer
    Pointer   = 1 << 1, // pointer type
    Ref       = 1 << 2, // reference type
    RValueRef = 1 << 3, // rvalue reference type
};
// TODO. 缓存函数，减少查找开销
struct SKR_CORE_API GenericType final : IGenericBase {
    SKR_RC_IMPL(override final)

    // ctor & dtor
    GenericType(not_null<RTTRType*> type, EGenericTypeFlag flag = EGenericTypeFlag::None);

    // getter
    EGenericTypeFlag flag() const;
    bool             is_const() const;
    bool             is_pointer() const;
    bool             is_ref() const;
    bool             is_rvalue_ref() const;
    bool             is_decayed_pointer() const;

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
private:
    RTTRType*        _type = nullptr;
    EGenericTypeFlag _flag = EGenericTypeFlag::None;

    // cached functions
    RTTRInvokerDefaultCtor _cached_default_ctor = {};
    DtorInvoker            _cached_dtor         = nullptr;
    RTTRInvokerCopyCtor    _cached_copy_ctor    = {};
    RTTRInvokerMoveCtor    _cached_move_ctor    = {};
    RTTRInvokerAssign      _cached_assign       = {};
    RTTRInvokerMoveAssign  _cached_move_assign  = {};
    RTTRInvokerEqual       _cached_equal        = {};
    RTTRInvokerHash        _cached_hash         = {};
    RTTRInvokerSwap        _cached_swap         = {};
};

// generic registry
using GenericProcessor = RC<IGenericBase> (*)(TypeSignatureView signature);
SKR_CORE_API void register_generic_processor(GUID generic_id, GenericProcessor processor);
SKR_CORE_API bool dry_build_generic(TypeSignatureView signature);
SKR_CORE_API RC<IGenericBase> build_generic(TypeSignatureView signature);
} // namespace skr