#pragma once
#include "SkrBase/config.h"
#include <type_traits>
#include <SkrContainers/optional.hpp>

// TODO. 添加基类以支持反射
// TODO. 添加类型擦除的调用 core 来支持脚本绑定
namespace skr
{
template <typename Func>
struct Delegate;
template <typename Func>
struct MemberFuncDelegateCore;
template <typename Func>
struct FunctorDelegateCore;
template <typename Func, typename Functor>
struct FunctorDelegateCoreNormal;
template <typename MemberFunc>
struct MemberFuncTraits;

enum class EDelegateKind : uint8_t
{
    Empty,
    Static,
    Member,
    Functor,
};

template <typename Obj, typename Ret, typename... Args>
struct MemberFuncTraits<Ret (Obj::*)(Args...)> {
    using FuncType                 = Ret (Obj::*)(Args...);
    using ObjType                  = Obj;
    using ObjPtrType               = Obj*;
    using RetType                  = Ret;
    static constexpr bool is_const = false;
};
template <typename Obj, typename Ret, typename... Args>
struct MemberFuncTraits<Ret (Obj::*)(Args...) const> {
    using FuncType                 = Ret (Obj::*)(Args...) const;
    using ObjType                  = Obj;
    using ObjPtrType               = const Obj*;
    using RetType                  = Ret;
    static constexpr bool is_const = true;
};

template <typename Ret, typename... Args>
struct MemberFuncDelegateCore<Ret(Args...)> {
    using InvokeFunc = Ret (*)(void*, Args...);
    using ThisType   = MemberFuncDelegateCore<Ret(Args...)>;

    void*      object = nullptr;
    InvokeFunc invoke = nullptr;

    template <auto MemberFunc>
    static ThisType Make(typename MemberFuncTraits<decltype(MemberFunc)>::ObjPtrType obj)
    {
        using Traits = MemberFuncTraits<decltype(MemberFunc)>;

        ThisType core;
        if constexpr (Traits::is_const)
        {
            core.object = const_cast<void*>(static_cast<const void*>(obj));
            core.invoke = [](void* obj, Args... args) -> Ret {
                return (static_cast<typename Traits::ObjPtrType>(obj)->*MemberFunc)(std::forward<Args>(args)...);
            };
        }
        else
        {
            core.object = obj;
            core.invoke = [](void* obj, Args... args) -> Ret {
                return (static_cast<typename Traits::ObjPtrType>(obj)->*MemberFunc)(std::forward<Args>(args)...);
            };
        }
        return core;
    }
};
template <typename Ret, typename... Args>
struct FunctorDelegateCore<Ret(Args...)> {
    using ThisType   = FunctorDelegateCore<Ret(Args...)>;
    using SizeType   = size_t;
    using InvokeFunc = Ret (*)(ThisType*, Args...);
    using DeleteFunc = void (*)(ThisType*);

    InvokeFunc invoke = nullptr;
    DeleteFunc dtor   = nullptr;

    inline SizeType ref_count() const
    {
        return _ref_count.load(std::memory_order_relaxed);
    }
    inline void add_ref()
    {
        _ref_count.fetch_add(1, std::memory_order_relaxed);
    }
    inline void release()
    {
        SKR_ASSERT(_ref_count.load(std::memory_order_relaxed) > 0);
        if (_ref_count.fetch_sub(1, std::memory_order_release) == 1)
        {
            std::atomic_thread_fence(std::memory_order_acquire);
            dtor(this);
            SkrDelete(this, SkrDeleteFlag_No_Dtor);
        }
    }

private:
    std::atomic<SizeType> _ref_count = 0;
};
template <typename Functor, typename Ret, typename... Args>
struct FunctorDelegateCoreNormal<Ret(Args...), Functor> : public FunctorDelegateCore<Ret(Args...)> {
    using Super = FunctorDelegateCore<Ret(Args...)>;

    Placeholder<Functor> functor_holder;

    inline FunctorDelegateCoreNormal(Functor&& functor)
        : FunctorDelegateCoreNormal()
    {
        new (functor_holder.data_typed()) Functor(std::move(functor));
    }
    inline FunctorDelegateCoreNormal(const Functor& functor)
        : FunctorDelegateCoreNormal()
    {
        new (functor_holder.data_typed()) Functor(std::move(functor));
    }

    inline FunctorDelegateCoreNormal()
    {
        Super::invoke = [](Super* core, Args... args) -> Ret {
            Functor* functor = static_cast<FunctorDelegateCoreNormal*>(core)->functor_holder.data_typed();
            return (*functor)(std::forward<Args>(args)...);
        };
        Super::dtor = [](Super* core) {
            Functor* functor = static_cast<FunctorDelegateCoreNormal*>(core)->functor_holder.data_typed();
            functor->~Functor();
        };
    }
};

template <typename Ret, typename... Args>
struct Delegate<Ret(Args...)> {
    using FuncType     = Ret(Args...);
    using MemberCore   = MemberFuncDelegateCore<FuncType>;
    using FunctorCore  = FunctorDelegateCore<FuncType>;
    using InvokeResult = std::conditional_t<std::is_same_v<Ret, void>, bool, Optional<Ret>>;

    // ctor & dtor
    Delegate();
    ~Delegate();

    // copy & move
    Delegate(const Delegate& other);
    Delegate(Delegate&& other);

    // assign & move assign
    Delegate& operator=(const Delegate& other);
    Delegate& operator=(Delegate&& other);

    // factory
    static Delegate Static(FuncType* func);
    template <auto MemberFunc>
    static Delegate Member(typename MemberFuncTraits<decltype(MemberFunc)>::ObjPtrType obj);
    template <typename Func>
    static Delegate Functor(Func&& functor);
    template <typename Func>
    static Delegate Lambda(Func&& lambda);
    static Delegate CustomFunctorCore(FunctorCore* core);

    // binder
    void bind_static(FuncType* func);
    template <auto MemberFunc>
    void bind_member(typename MemberFuncTraits<decltype(MemberFunc)>::ObjPtrType obj);
    template <typename Func>
    void bind_functor(Func&& functor);
    template <typename Func>
    void bind_lambda(Func&& lambda);
    void bind_custom_functor_core(FunctorCore* core);

    // invoke
    InvokeResult invoke(Args... args);

    // ops
    void reset();

    // getter
    EDelegateKind kind() const;
    bool          is_empty() const;

private:
    union
    {
        FuncType*    _static_delegate;  // 64 bits,  void*
        MemberCore   _member_delegate;  // 128 bits, [void*, void*]
        FunctorCore* _functor_delegate; // 64 bits,  void*
    };
    EDelegateKind _kind = EDelegateKind::Empty; // 8 bits, wrap to 64 bits
};
} // namespace skr

namespace skr
{
// ctor & dtor
template <typename Ret, typename... Args>
inline Delegate<Ret(Args...)>::Delegate()
{
}
template <typename Ret, typename... Args>
inline Delegate<Ret(Args...)>::~Delegate()
{
    reset();
}

// copy & move
template <typename Ret, typename... Args>
inline Delegate<Ret(Args...)>::Delegate(const Delegate& other)
{
    _kind = other._kind;
    switch (_kind)
    {
    case EDelegateKind::Empty:
        break;
    case EDelegateKind::Static:
        _static_delegate = other._static_delegate;
        break;
    case EDelegateKind::Member:
        _member_delegate = other._member_delegate;
        break;
    case EDelegateKind::Functor:
        _functor_delegate = other._functor_delegate;
        _functor_delegate->add_ref();
        break;
    }
}
template <typename Ret, typename... Args>
inline Delegate<Ret(Args...)>::Delegate(Delegate&& other)
{
    _kind = other._kind;
    switch (_kind)
    {
    case EDelegateKind::Empty:
        break;
    case EDelegateKind::Static:
        _static_delegate = other._static_delegate;
        break;
    case EDelegateKind::Member:
        _member_delegate = other._member_delegate;
        break;
    case EDelegateKind::Functor:
        _functor_delegate       = other._functor_delegate;
        other._functor_delegate = nullptr;
        break;
    }
    other._kind = EDelegateKind::Empty;
}

// assign & move assign
template <typename Ret, typename... Args>
inline Delegate<Ret(Args...)>& Delegate<Ret(Args...)>::operator=(const Delegate& other)
{
    reset();
    _kind = other._kind;
    switch (_kind)
    {
    case EDelegateKind::Empty:
        break;
    case EDelegateKind::Static:
        _static_delegate = other._static_delegate;
        break;
    case EDelegateKind::Member:
        _member_delegate = other._member_delegate;
        break;
    case EDelegateKind::Functor:
        _functor_delegate = other._functor_delegate;
        _functor_delegate->add_ref();
        break;
    }

    return *this;
}
template <typename Ret, typename... Args>
inline Delegate<Ret(Args...)>& Delegate<Ret(Args...)>::operator=(Delegate&& other)
{
    reset();
    _kind = other._kind;
    switch (_kind)
    {
    case EDelegateKind::Empty:
        break;
    case EDelegateKind::Static:
        _static_delegate = other._static_delegate;
        break;
    case EDelegateKind::Member:
        _member_delegate = other._member_delegate;
        break;
    case EDelegateKind::Functor:
        _functor_delegate       = other._functor_delegate;
        other._functor_delegate = nullptr;
        break;
    }
    other._kind = EDelegateKind::Empty;

    return *this;
}

// factory
template <typename Ret, typename... Args>
inline Delegate<Ret(Args...)> Delegate<Ret(Args...)>::Static(FuncType* func)
{
    Delegate delegate;
    delegate.bind_static(func);
    return delegate;
}
template <typename Ret, typename... Args>
template <auto MemberFunc>
inline Delegate<Ret(Args...)> Delegate<Ret(Args...)>::Member(typename MemberFuncTraits<decltype(MemberFunc)>::ObjPtrType obj)
{
    Delegate delegate;
    delegate.bind_member<MemberFunc>(obj);
    return delegate;
}
template <typename Ret, typename... Args>
template <typename Func>
inline Delegate<Ret(Args...)> Delegate<Ret(Args...)>::Functor(Func&& functor)
{
    Delegate delegate;
    delegate.bind_functor(std::forward<Func>(functor));
    return delegate;
}
template <typename Ret, typename... Args>
template <typename Func>
inline Delegate<Ret(Args...)> Delegate<Ret(Args...)>::Lambda(Func&& lambda)
{
    Delegate delegate;
    delegate.bind_lambda(std::forward<Func>(lambda));
    return delegate;
}
template <typename Ret, typename... Args>
inline Delegate<Ret(Args...)> Delegate<Ret(Args...)>::CustomFunctorCore(FunctorCore* core)
{
    Delegate delegate;
    delegate.bind_custom_functor_core(core);
    return delegate;
}

// binder
template <typename Ret, typename... Args>
inline void Delegate<Ret(Args...)>::bind_static(FuncType* func)
{
    reset();
    _static_delegate = func;
    _kind            = EDelegateKind::Static;
}
template <typename Ret, typename... Args>
template <auto MemberFunc>
inline void Delegate<Ret(Args...)>::bind_member(typename MemberFuncTraits<decltype(MemberFunc)>::ObjPtrType obj)
{
    reset();
    _member_delegate = MemberCore::template Make<MemberFunc>(obj);
    _kind            = EDelegateKind::Member;
}
template <typename Ret, typename... Args>
template <typename Func>
inline void Delegate<Ret(Args...)>::bind_functor(Func&& functor)
{
    using FunctorCore = FunctorDelegateCoreNormal<FuncType, std::remove_reference_t<Func>>;

    reset();
    auto* core = SkrNew<FunctorCore>(std::forward<Func>(functor));
    core->add_ref();
    _functor_delegate = core;
    _kind             = EDelegateKind::Functor;
}
template <typename Ret, typename... Args>
template <typename Func>
inline void Delegate<Ret(Args...)>::bind_lambda(Func&& lambda)
{
    using FunctorCore = FunctorDelegateCoreNormal<FuncType, std::remove_reference_t<Func>>;

    reset();
    auto* core = SkrNew<FunctorCore>(std::forward<Func>(lambda));
    core->add_ref();
    _functor_delegate = core;
    _kind             = EDelegateKind::Functor;
}
template <typename Ret, typename... Args>
inline void Delegate<Ret(Args...)>::bind_custom_functor_core(FunctorCore* core)
{
    reset();
    _functor_delegate = core;
    _functor_delegate->add_ref();
    _kind = EDelegateKind::Functor;
}

// invoke
template <typename Ret, typename... Args>
inline typename Delegate<Ret(Args...)>::InvokeResult Delegate<Ret(Args...)>::invoke(Args... args)
{
    if constexpr (std::is_same_v<Ret, void>)
    {
        switch (_kind)
        {
        case EDelegateKind::Empty:
            return false;
        case EDelegateKind::Static:
            _static_delegate(std::forward<Args>(args)...);
            return true;
        case EDelegateKind::Member:
            _member_delegate.invoke(_member_delegate.object, std::forward<Args>(args)...);
            return true;
        case EDelegateKind::Functor:
            _functor_delegate->invoke(_functor_delegate, std::forward<Args>(args)...);
            return true;
        };
        return false;
    }
    else
    {
        switch (_kind)
        {
        case EDelegateKind::Empty:
            return {};
        case EDelegateKind::Static:
            return _static_delegate(std::forward<Args>(args)...);
        case EDelegateKind::Member:
            return _member_delegate.invoke(_member_delegate.object, std::forward<Args>(args)...);
        case EDelegateKind::Functor:
            return _functor_delegate->invoke(_functor_delegate, std::forward<Args>(args)...);
        };
        return {};
    }
}

// ops
template <typename Ret, typename... Args>
inline void Delegate<Ret(Args...)>::reset()
{
    switch (_kind)
    {
    case EDelegateKind::Empty:
    case EDelegateKind::Static:
    case EDelegateKind::Member:
        break;
    case EDelegateKind::Functor:
        _functor_delegate->release();
        break;
    }
    _kind = EDelegateKind::Empty;
}

// getter
template <typename Ret, typename... Args>
inline EDelegateKind Delegate<Ret(Args...)>::kind() const
{
    return _kind;
}
template <typename Ret, typename... Args>
inline bool Delegate<Ret(Args...)>::is_empty() const
{
    return _kind == EDelegateKind::Empty;
}
} // namespace skr