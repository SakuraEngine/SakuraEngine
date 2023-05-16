#pragma once
#include <type_traits>
#include <memory>

namespace skr
{
// type as value
template<class T>
struct type_t{ using type = T; };
// typelist via function type
#define SKR_TYPELIST(...) skr::type_t<void(__VA_ARGS__)>{}

template<class F> struct validator
{
    template<class T> constexpr validator(T&&) noexcept {}
    // by variable
    template<class ...Args> constexpr bool operator()(Args&&...) const noexcept
    {
        return std::is_invocable_v<F&&, Args&&...>;
    }
    // by type
    template<class ...Args> constexpr bool operator()(type_t<void(Args...)>) const noexcept
    {
        return std::is_invocable_v<F&&, Args&&...>;
    }
};
template<class F> validator(F&&)->validator<F&&>;
#define SKR_VALIDATOR(def, ...) ::skr::validator{[]def -> decltype(__VA_ARGS__) { return __VA_ARGS__; }}

// use sizeof to check if a type is complete
constexpr static auto is_complete = SKR_VALIDATOR((auto t), sizeof(t));
template<class T>
constexpr bool is_complete_v = is_complete(SKR_TYPELIST(T));

#if __cpp_lib_assume_aligned

using std::assume_aligned;

#else

template<std::size_t N, class T>
[[nodiscard]] constexpr T* assume_aligned(T* ptr)
{
#if defined(__clang__) || defined(__GNUC__) || !defined(__ICC)
    return reinterpret_cast<T*>(__builtin_assume_aligned(ptr, N));
#elif defined(_MSC_VER)
    if (reinterpret_cast<std::uintptr_t>(ptr) & -static_cast<std::intptr_t>(N) == 0)
        return ptr;
    else
        assume(0);
#elif defined(__ICC)
    switch (N) {
        case 2: __assume_aligned(ptr, 2); break;
        case 4: __assume_aligned(ptr, 4); break;
        case 8: __assume_aligned(ptr, 8); break;
        case 16: __assume_aligned(ptr, 16); break;
        case 32: __assume_aligned(ptr, 32); break;
        case 64: __assume_aligned(ptr, 64); break;
        case 128: __assume_aligned(ptr, 128); break;
    }
    return ptr;
#else // unknown compiler
    return ptr;
#endif
}

#endif
}