#pragma once
#include <type_traits>
#include <memory>

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
class FunctionRef;

/// Specialization for function types.
template <class R, class... Args>
class FunctionRef<R(Args...)>
{
public:
    constexpr FunctionRef() noexcept = delete;
    constexpr FunctionRef(std::nullptr_t) noexcept {}

    /// Creates a `FunctionRef` which refers to the same callable as `rhs`.
    constexpr FunctionRef(const FunctionRef<R(Args...)>& rhs) noexcept = default;

    /// Constructs a `FunctionRef` referring to `f`.
    ///
    /// \synopsis template <typename F> constexpr FunctionRef(F &&f) noexcept
    template <typename F,
    std::enable_if_t<
    !std::is_same<std::decay_t<F>, FunctionRef>::value &&
    std::is_invocable_r<R, F&&, Args...>::value>* = nullptr>
    constexpr FunctionRef(F&& f) noexcept
        : obj_(const_cast<void*>(reinterpret_cast<const void*>(std::addressof(f))))
    {
        callback_ = [](void* obj, Args... args) -> R {
            return std::invoke(
            *reinterpret_cast<typename std::add_pointer<F>::type>(obj),
            std::forward<Args>(args)...);
        };
    }

    /// Makes `*this` refer to the same callable as `rhs`.
    constexpr FunctionRef<R(Args...)>&
    operator=(const FunctionRef<R(Args...)>& rhs) noexcept = default;

    explicit operator bool() const noexcept { return callback_ != nullptr; }

    /// Makes `*this` refer to `f`.
    ///
    /// \synopsis template <typename F> constexpr FunctionRef &operator=(F &&f) noexcept;
    template <typename F,
    std::enable_if_t<std::is_invocable_r<R, F&&, Args...>::value>* = nullptr>
    constexpr FunctionRef<R(Args...)>& operator=(F&& f) noexcept
    {
        obj_ = reinterpret_cast<void*>(std::addressof(f));
        callback_ = [](void* obj, Args... args) {
            return std::invoke(
            *reinterpret_cast<typename std::add_pointer<F>::type>(obj),
            std::forward<Args>(args)...);
        };

        return *this;
    }

    /// Swaps the referred callables of `*this` and `rhs`.
    constexpr void swap(FunctionRef<R(Args...)>& rhs) noexcept
    {
        std::swap(obj_, rhs.obj_);
        std::swap(callback_, rhs.callback_);
    }

    /// Call the stored callable with the given arguments.
    R operator()(Args... args) const
    {
        return callback_(obj_, std::forward<Args>(args)...);
    }

private:
    void* obj_ = nullptr;
    R (*callback_)(void*, Args...) = nullptr;
};

template <class F, class = void>
struct FunctionTrait : public FunctionTrait<decltype(&F::operator())> {};

template <class R, class... Args>
struct FunctionTrait<R(Args...)> {
    using raw = R(Args...);
};

template <class T, class R, class... Args>
struct FunctionTrait<R (T::*)(Args...)> {
    using raw = R(Args...);
};

template <class T, class R, class... Args>
struct FunctionTrait<R (T::*)(Args...) noexcept(true)> {
    using raw = R(Args...);
};

template <class T, class R, class... Args>
struct FunctionTrait<R (T::*)(Args...) const> {
    using raw = R(Args...);
};

template <class T, class R, class... Args>
struct FunctionTrait<R (T::*)(Args...) const noexcept(true)> {
    using raw = R(Args...);
};

template <class R, class... Args>
struct FunctionTrait<R (*)(Args...)> {
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