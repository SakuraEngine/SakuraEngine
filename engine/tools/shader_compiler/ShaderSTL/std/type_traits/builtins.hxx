#pragma once
#include "../attributes.hxx"

using int16 = short;
using uint16_t = unsigned short;
using uint16 = uint16_t;
using int32_t = int;
using int32 = int32_t;
using int64_t = long long;
using int64 = int64_t;
using uint32_t = unsigned int;
using uint32 = uint32_t;
using uint = uint32_t;
using uint64_t = unsigned long long;
using uint64 = uint64_t;

struct [[builtin("half")]] half
{
    [[ignore]] half() = default;
    [[ignore]] half(float);
    [[ignore]] half(uint32);
    [[ignore]] half(int32);

private:
    short v;
};

namespace cppsl
{

template <class T, T v>
struct integral_constant
{
    static constexpr T value = v;

    using value_type = T;
    using type = integral_constant;

    constexpr operator value_type() const noexcept
    {
        return value;
    }

    [[nodiscard]] constexpr value_type operator()() const noexcept
    {
        return value;
    }
};

template <class...>
using void_t = void;

template <bool B, class T, class F>
struct conditional
{
    using type = T;
};
template <class T, class F>
struct conditional<false, T, F>
{
    using type = F;
};

template <bool b, class T, class F>
using conditional_t = typename conditional<b, T, F>::type;

template <bool v>
using bool_constant = integral_constant<bool, v>;

using true_type = bool_constant<true>;
using false_type = bool_constant<false>;

template <class T, class U>
constexpr bool is_same_v = __is_same(T, U);

template <class T>
inline constexpr bool is_void_v = __is_void(T);

template <class T>
inline constexpr bool is_member_object_pointer_v = __is_member_object_pointer(T);

template <class T, class U>
struct is_same : bool_constant<__is_same(T, U)>
{
};

template <class T>
struct remove_cv
{ // remove top-level const and volatile qualifiers
    using type = T;

    template <template <class> class F>
    using apply = F<T>; // apply cv-qualifiers from the class template argument to F<T>
};

template <class T>
struct remove_cv<const T>
{
    using type = T;

    template <template <class> class F>
    using apply = const F<T>;
};

template <class T>
struct remove_cv<volatile T>
{
    using type = T;

    template <template <class> class F>
    using apply = volatile F<T>;
};

template <class T>
struct remove_cv<const volatile T>
{
    using type = T;

    template <template <class> class F>
    using apply = const volatile F<T>;
};

template <class T>
using remove_cv_t = typename remove_cv<T>::type;

// reference modifications
template <class T>
struct remove_reference
{
    using type = __remove_reference_t(T);
};

template <class T>
using remove_reference_t = __remove_reference_t(T);

template <class T, class = void>
struct add_reference
{ // add reference (non-referenceable type)
    using lvalue = T;
    using rvalue = T;
};

template <class T>
struct add_reference<T, void_t<T&>>
{ // (referenceable type)
    using lvalue = T&;
    using rvalue = T&&;
};

template <class T>
struct add_lvalue_reference
{
    using type = typename add_reference<T>::lvalue;
};
template <class T>
struct add_rvalue_reference
{
    using type = typename add_reference<T>::rvalue;
};
template <class T>
using add_lvalue_reference_t = typename add_lvalue_reference<T>::type;
template <class T>
using add_rvalue_reference_t = typename add_rvalue_reference<T>::type;

template <class T>
struct remove_cvref
{
    using type = remove_cv_t<remove_reference_t<T>>;
};

template <class T>
using remove_cvref_t = typename remove_cvref<T>::type;

template <class T, class... Ts>
constexpr bool is_any_of_v = (is_same_v<T, Ts> || ...);

template <class T>
constexpr bool is_integral_v = is_any_of_v<remove_cv_t<T>, bool, char, signed char, unsigned char, short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long>;

template <class T>
[[ignore]] add_rvalue_reference_t<T> declval() noexcept /* as unevaluated operand*/
{
    static_assert(false, "Calling declval is ill-formed, see N4950 [declval]/2.");
}

} // namespace cppsl