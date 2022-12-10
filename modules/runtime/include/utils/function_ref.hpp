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
/// void foo (function_ref<int(int)> func) {
///     std::cout << "Result is " << func(21); //42
/// }
///
/// foo([](int i) { return i*2; });
template <class F>
class function_ref;

/// Specialization for function types.
template <class R, class... Args>
class function_ref<R(Args...)>
{
public:
    constexpr function_ref() noexcept = delete;
    constexpr function_ref(std::nullptr_t) noexcept {}

    /// Creates a `function_ref` which refers to the same callable as `rhs`.
    constexpr function_ref(const function_ref<R(Args...)>& rhs) noexcept = default;

    /// Constructs a `function_ref` referring to `f`.
    ///
    /// \synopsis template <typename F> constexpr function_ref(F &&f) noexcept
    template <typename F,
    std::enable_if_t<
    !std::is_same<std::decay_t<F>, function_ref>::value &&
    std::is_invocable_r<R, F&&, Args...>::value>* = nullptr>
    constexpr function_ref(F&& f) noexcept
        : obj_(const_cast<void*>(reinterpret_cast<const void*>(std::addressof(f))))
    {
        callback_ = [](void* obj, Args... args) -> R {
            return std::invoke(
            *reinterpret_cast<typename std::add_pointer<F>::type>(obj),
            std::forward<Args>(args)...);
        };
    }

    /// Makes `*this` refer to the same callable as `rhs`.
    constexpr function_ref<R(Args...)>&
    operator=(const function_ref<R(Args...)>& rhs) noexcept = default;

    explicit operator bool() const noexcept { return callback_ != nullptr; }

    /// Makes `*this` refer to `f`.
    ///
    /// \synopsis template <typename F> constexpr function_ref &operator=(F &&f) noexcept;
    template <typename F,
    std::enable_if_t<std::is_invocable_r<R, F&&, Args...>::value>* = nullptr>
    constexpr function_ref<R(Args...)>& operator=(F&& f) noexcept
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
    constexpr void swap(function_ref<R(Args...)>& rhs) noexcept
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
    R (*callback_)
    (void*, Args...) = nullptr;
};

/// Swaps the referred callables of `lhs` and `rhs`.
template <typename R, typename... Args>
constexpr void swap(function_ref<R(Args...)>& lhs,
function_ref<R(Args...)>& rhs) noexcept
{
    lhs.swap(rhs);
}

template <class F, class = void>
struct function_trait : public function_trait<decltype(&F::operator())> {};

template <class R, class... Args>
struct function_trait<R(Args...)> {
    using raw = R(Args...);
};

template <class T, class R, class... Args>
struct function_trait<R (T::*)(Args...)> {
    using raw = R(Args...);
};

template <class T, class R, class... Args>
struct function_trait<R (T::*)(Args...) noexcept(true)> {
    using raw = R(Args...);
};

template <class T, class R, class... Args>
struct function_trait<R (T::*)(Args...) const> {
    using raw = R(Args...);
};

template <class T, class R, class... Args>
struct function_trait<R (T::*)(Args...) const noexcept(true)> {
    using raw = R(Args...);
};

template <class R, class... Args>
struct function_trait<R (*)(Args...)> {
    using raw = R(Args...);
};

template <template <class... T> class Tmp, class F>
struct map;

template <template <class... T> class Tmp, class... Args>
struct map<Tmp, void(Args...)> {
    using type = Tmp<Args...>;
};

template <template <class... T> class Tmp, class F>
using map_t = typename map<Tmp, F>::type;

template <typename F>
function_ref(F&&) -> function_ref<typename function_trait<std::remove_reference_t<F>>::raw>;
} // namespace skr