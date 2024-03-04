#pragma once
#include "SkrBase/config.h"
#include <type_traits>
#include <concepts>

// map functor
namespace skr
{
template <typename T>
struct MapFwd {

    SKR_INLINE constexpr T&       operator()(T& v) const { return v; }
    SKR_INLINE constexpr const T& operator()(const T& v) const { return v; }
};
} // namespace skr

// compare functors
namespace skr
{
#define SKR_DEF_COMPARE_FUNCTOR(__NAME, __OP)                              \
    template <typename T = void>                                           \
    struct __NAME {                                                        \
        SKR_INLINE constexpr bool operator()(const T& a, const T& b) const \
        {                                                                  \
            return a __OP b;                                               \
        }                                                                  \
    };                                                                     \
    template <>                                                            \
    struct __NAME<void> {                                                  \
        template <typename A, typename B>                                  \
        SKR_INLINE constexpr bool operator()(A&& a, B&& b) const           \
        {                                                                  \
            return std::forward<A>(a) __OP std::forward<B>(b);             \
        }                                                                  \
    };

SKR_DEF_COMPARE_FUNCTOR(Less, <)
SKR_DEF_COMPARE_FUNCTOR(Greater, >)
SKR_DEF_COMPARE_FUNCTOR(LessEqual, <=)
SKR_DEF_COMPARE_FUNCTOR(GreaterEqual, >=)
SKR_DEF_COMPARE_FUNCTOR(Equal, ==)
SKR_DEF_COMPARE_FUNCTOR(NotEqual, !=)

#undef SKR_DEF_COMPARE_FUNCTOR
} // namespace skr

// arithmetic functors
namespace skr
{
#define SKR_DEF_ARITHMETIC_FUNCTOR(__NAME, __OP)                                 \
    template <typename T = void>                                                 \
    struct __NAME {                                                              \
        SKR_INLINE constexpr T operator()(const T& a, const T& b)                \
        {                                                                        \
            return a __OP b;                                                     \
        }                                                                        \
    };                                                                           \
    template <>                                                                  \
    struct __NAME<void> {                                                        \
        template <typename A, typename B>                                        \
        SKR_INLINE constexpr auto operator()(A&& a, B&& b) -> decltype(a __OP b) \
        {                                                                        \
            return a __OP b;                                                     \
        }                                                                        \
    };

SKR_DEF_ARITHMETIC_FUNCTOR(OpAdd, +)
SKR_DEF_ARITHMETIC_FUNCTOR(OpSub, -)
SKR_DEF_ARITHMETIC_FUNCTOR(OpMul, *)
SKR_DEF_ARITHMETIC_FUNCTOR(OpDov, /)

#undef SKR_DEF_ARITHMETIC_FUNCTOR
} // namespace skr

// concept
namespace skr
{
template <typename Functor, typename LHS, typename RHS>
concept BinaryFunctorCallable = requires(Functor f, LHS lhs, RHS rhs) {
    f(lhs, rhs);
};
template <typename Functor, typename LHS, typename RHS, typename Ret>
concept BinaryFunctorCallableWithRet = requires(Functor f, LHS lhs, RHS rhs) {
    {
        f(lhs, rhs)
    } -> std::convertible_to<Ret>;
};
template <typename Functor, typename T>
concept UnaryFunctorCallable = requires(Functor f, T t) {
    f(t);
};
template <typename Functor, typename T, typename Ret>
concept UnaryFunctorCallableWithRet = requires(Functor f, T t) {
    {
        f(t)
    } -> std::convertible_to<Ret>;
};
} // namespace skr

// gcd & lcm
namespace skr::algo
{
// greatest common divisor (gcd)
template <typename T>
SKR_INLINE T gcd(T a, T b)
{
    while (b != 0)
    {
        T tmp = b;
        b     = a % b;
        a     = tmp;
    }
    return a;
}

// lowest common multiple (lcm)
template <typename T>
SKR_INLINE T lcm(T a, T b)
{
    T gcd_val = gcd(a, b);
    return a * b / gcd_val;
}

// is sorted
template <typename T, typename TP = Less<>>
SKR_INLINE bool is_sorted(T begin, T end, TP p = {})
{
    if (begin < end)
    {
        T next = begin + 1;
        while (next != end)
        {
            if (p(*next, *begin))
            {
                return false;
            }

            ++begin;
            ++next;
        }
    }
    return true;
}
} // namespace skr::algo