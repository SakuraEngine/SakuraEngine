#pragma once
#include <type_traits>

namespace skr
{
    // type as value
    template<class T>
    struct type_t{ using type = T; };
    // typelist via function type
    #define SKR_TYPELIST(...) type_t<void(__VA_ARGS__)>{}

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
}