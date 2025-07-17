#pragma once
#include <type_traits>
#include <stdio.h>

namespace skr
{
// type as value
template <class T>
struct type_t {
    using type = T;
};
// typelist via function type
#define SKR_TYPELIST(...) \
    skr::type_t<void(__VA_ARGS__)> {}

// validate
//?NOTE: deprecated, use c++20 concept will be better
#if 0
template <class F>
struct validator {
    template <class T>
    constexpr validator(T&&) noexcept {}
    // by variable
    template <class... Args>
    constexpr bool operator()(Args&&...) const noexcept
    {
        return std::is_invocable_v<F&&, Args&&...>;
    }
    // by type
    template <class... Args>
    constexpr bool operator()(type_t<void(Args...)>) const noexcept
    {
        return std::is_invocable_v<F&&, Args&&...>;
    }
};
template <class F>
validator(F&&) -> validator<F&&>;
    #define SKR_VALIDATOR(def, ...)                                 \
        ::skr::validator                                            \
        {                                                           \
            [] def -> decltype(__VA_ARGS__) { return __VA_ARGS__; } \
        }
#endif

// misc traits
//?NOTE: deprecated, use c++20 concept will be better
#if 0
// use sizeof to check if a type is complete
constexpr static auto is_complete = SKR_VALIDATOR((auto t), sizeof(t));
template <class T>
constexpr bool is_complete_v = is_complete(SKR_TYPELIST(T));

template <typename T>
inline constexpr bool is_carray_v = false;

template <typename T, std::size_t N>
inline constexpr bool is_carray_v<T[N]> = true;

template <typename T, template <typename...> typename Template>
inline constexpr bool is_specialization_v = false; // true if and only if T is a specialization of Template

template <template <typename...> typename Template, typename... Args>
inline constexpr bool is_specialization_v<Template<Args...>, Template> = true;

template <typename ChildType, template <class...> class Template, typename... Args>
inline constexpr bool is_convertible_to_specialization_v = std::is_convertible_v<ChildType*, Template<Args...>*>;

template <template <class...> class ChildTemplate, template <class...> class Template, typename... Args>
inline constexpr bool is_convertible_to_specialization_v<ChildTemplate<Args...>, Template> =
    std::is_convertible_v<ChildTemplate<Args...>*, Template<Args...>*>;
#endif

// is detected
//?NOTE: deprecated, use c++20 concept will be better
#if 0
namespace detail
{
template <typename, template <typename...> class Op, typename... T>
struct is_detected_impl : std::false_type {
};
template <template <typename...> class Op, typename... T>
struct is_detected_impl<std::void_t<Op<T...>>, Op, T...> : std::true_type {
};
} // namespace detail

template <template <typename...> class Op, typename... T>
using is_detected = detail::is_detected_impl<void, Op, T...>;
template <template <typename...> class Op, typename... T>
inline constexpr bool is_detected_v = is_detected<Op, T...>::value;
#endif

template <std::size_t N, class T>
[[nodiscard]] constexpr T* assume_aligned(T* ptr)
{
#if defined(__clang__) || (defined(__GNUC__) && !defined(__ICC))
    return reinterpret_cast<T*>(__builtin_assume_aligned(ptr, N));
#elif defined(_MSC_VER)
    if ((reinterpret_cast<std::uintptr_t>(ptr) & -static_cast<std::intptr_t>(N)) == 0)
        return ptr;
    else
        __assume(0);
#elif defined(__ICC)
    switch (N)
    {
    case 2:
        __assume_aligned(ptr, 2);
        break;
    case 4:
        __assume_aligned(ptr, 4);
        break;
    case 8:
        __assume_aligned(ptr, 8);
        break;
    case 16:
        __assume_aligned(ptr, 16);
        break;
    case 32:
        __assume_aligned(ptr, 32);
        break;
    case 64:
        __assume_aligned(ptr, 64);
        break;
    case 128:
        __assume_aligned(ptr, 128);
        break;
    }
    return ptr;
#else // unknown compiler
    #define SKR_USE_FALLBACK_ASSUME_ALIGNED
    return ptr;
#endif
}

#ifdef SKR_USE_FALLBACK_ASSUME_ALIGNED
} // namespace skr

    #include <memory>
namespace skr
{
    #if __cpp_lib_assume_aligned
using std::assume_aligned;
    #endif
#endif
} // namespace skr