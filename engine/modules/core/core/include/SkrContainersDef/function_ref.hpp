#pragma once
#include <type_traits>
#include <memory>
#include <exception>
#include <SkrBase/template/concepts.hpp>
#include <SkrRTTR/script/stack_proxy.hpp>
#include <SkrContainersDef/span.hpp>

namespace skr
{
/// A lightweight non-owning reference to a callable.
///
/// Example usage:
///
/// ```cpp
/// void foo (FunctionRef<int(int)> func) {
///     std::cout << "Result is " << func(21); //42
/// }
///
/// foo([](int i) { return i*2; });
template <class F>
struct FunctionRef;

enum class EFunctionRefKind
{
    Empty,
    Caller,
    StackProxy
};

/// Specialization for function types.
template <class R, class... Args>
struct FunctionRef<R(Args...)>
{
    using FuncType = R(Args...);
    using CallerType = R(void*, Args...);
    using StackProxyCallerType = void(void* payload,Span<const StackProxy> params, StackProxy return_value);

    template <typename T>
    [[noreturn]] static T unreachable_return()
    {
        SKR_ASSERT(false && "unreachable control path in FunctionRef::operator()");
#if defined(_MSC_VER)
        __assume(0);
#elif defined(__GNUC__)
        __builtin_unreachable();
#endif
        std::terminate();
    }

    // ctor & dtor
    inline FunctionRef() noexcept = default;
    inline FunctionRef(std::nullptr_t) noexcept {}
    template <concepts::InvocableR<R, Args...> Func>
    inline FunctionRef(Func&& f) noexcept
        : _kind(EFunctionRefKind::Caller)
        , _payload(const_cast<void*>(reinterpret_cast<const void*>(std::addressof(f))))
    {
        _caller = (void*)+[](void* obj, Args... args) -> R {
            return std::invoke(
                *reinterpret_cast<typename std::add_pointer_t<Func>>(obj),
                std::forward<Args>(args)...);
        };
    }
    inline ~FunctionRef() noexcept = default;

    // copy & move
    inline FunctionRef(const FunctionRef& rhs) noexcept = default;
    inline FunctionRef(FunctionRef&& rhs) noexcept = default;

    // assign & move assign
    inline FunctionRef& operator=(const FunctionRef& rhs) noexcept = default;
    inline FunctionRef& operator=(FunctionRef&& rhs) noexcept = default;
    template <concepts::InvocableR<R, Args...> Func>
    inline FunctionRef& operator=(Func&& f)
    {
        bind_functor(std::forward<Func>(f));
        return *this;
    }

    // bind
    template <concepts::InvocableR<R, Args...> Func>
    inline void bind_functor(Func&& f)
    {
        _kind = EFunctionRefKind::Caller;
        _payload = const_cast<void*>(reinterpret_cast<const void*>(std::addressof(f)));
        _caller = (void*)+[](void* obj, Args... args) -> R {
            return std::invoke(
                *reinterpret_cast<typename std::add_pointer_t<Func>>(obj),
                std::forward<Args>(args)...);
        };
    }
    template <concepts::Invocable<Span<const StackProxy>, StackProxy> Func>
    inline void bind_stack_proxy(Func&& f)
    {
        _kind = EFunctionRefKind::StackProxy;
        _payload = const_cast<void*>(reinterpret_cast<const void*>(std::addressof(f)));
        _caller = (void*)+[](void* obj, Span<const StackProxy> params, StackProxy return_value) {
            std::invoke(
                *reinterpret_cast<typename std::add_pointer_t<Func>>(obj),
                params,
                return_value);
        };
    }
    inline void reset() noexcept
    {
        _kind = EFunctionRefKind::Empty;
        _payload = nullptr;
        _caller = nullptr;
    }

    // validate
    inline bool is_valid() const noexcept
    {
        return _caller != nullptr;
    }
    inline bool is_empty() const noexcept
    {
        return !is_valid();
    }
    inline explicit operator bool() const noexcept
    {
        return is_valid();
    }

    // swap
    inline void swap(FunctionRef<R(Args...)>& rhs) noexcept
    {
        std::swap(_payload, rhs._payload);
        std::swap(_caller, rhs._caller);
    }

    /// Call the stored callable with the given arguments.
    inline R operator()(Args... args) const
    {
        switch (_kind)
        {
        case EFunctionRefKind::Empty:
            SKR_ASSERT(false && "FunctionRef is empty");
            // return {};
        case EFunctionRefKind::Caller:
            return reinterpret_cast<CallerType*>(_caller)(
                reinterpret_cast<void*>(_payload),
                std::forward<Args>(args)...);
        case EFunctionRefKind::StackProxy: {
            if constexpr (concepts::WithTypeSignatureTraits<FuncType>)
            {
                if constexpr (std::is_same_v<R, void>)
                {
                    reinterpret_cast<StackProxyCallerType*>(_caller)(
                        _payload,
                        { StackProxyMaker<Args>::Make(std::forward<Args>(args))... },
                        {});
                }
                else
                {
                    Placeholder<R> result;
                    reinterpret_cast<StackProxyCallerType*>(_caller)(
                        _payload,
                        { StackProxyMaker<Args>::Make(std::forward<Args>(args))... },
                        { .data = result.data(), .signature = type_signature_of<R>() });
                    return std::move(*result.data_typed());
                }
            }
            else
            {
                SKR_ASSERT(false && "cannot use stack proxy function ref without type signature support");
                // return {};
            }
        }
        }
        // Fallback to satisfy all control paths for both void and non-void R
        if constexpr (std::is_void_v<R>)
            return;
        else
            return unreachable_return<std::remove_reference_t<R>>();
    }

private:
    EFunctionRefKind _kind = EFunctionRefKind::Empty;
    void* _payload = nullptr;
    void* _caller = nullptr;
};

struct FunctionRefMemory
{
    EFunctionRefKind _kind = EFunctionRefKind::Empty;
    void* _payload = nullptr;
    void* _caller = nullptr;
};

template <class F, class = void>
struct FunctionTrait : public FunctionTrait<decltype(&F::operator())>
{
};

template <class R, class... Args>
struct FunctionTrait<R(Args...)>
{
    using raw = R(Args...);
};

template <class T, class R, class... Args>
struct FunctionTrait<R (T::*)(Args...)>
{
    using raw = R(Args...);
};

template <class T, class R, class... Args>
struct FunctionTrait<R (T::*)(Args...) noexcept(true)>
{
    using raw = R(Args...);
};

template <class T, class R, class... Args>
struct FunctionTrait<R (T::*)(Args...) const>
{
    using raw = R(Args...);
};

template <class T, class R, class... Args>
struct FunctionTrait<R (T::*)(Args...) const noexcept(true)>
{
    using raw = R(Args...);
};

template <class R, class... Args>
struct FunctionTrait<R (*)(Args...)>
{
    using raw = R(Args...);
};

/* ?
template <template <class... T> class Tmp, class F>
struct map;

template <template <class... T> class Tmp, class... Args>
struct map<Tmp, void(Args...)> {
    using type = Tmp<Args...>;
};

template <template <class... T> class Tmp, class F>
using map_t = typename map<Tmp, F>::type;
*/

template <typename F>
FunctionRef(F&&) -> FunctionRef<typename FunctionTrait<std::remove_reference_t<F>>::raw>;
} // namespace skr