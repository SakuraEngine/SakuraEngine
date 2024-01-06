#pragma once
#include <type_traits>

// clang-format off
namespace skr {
namespace detail {
    struct _Empty {};

    template <class Derived, class U>
    constexpr bool _StaticAssertNonInherited() noexcept {
        static_assert(
            !std::is_base_of<U, Derived>::value,
            "====================> count_member: Inherited types are not supported."
        );
        return true;
    }

    template <typename Derived>
    struct _Any 
    {
        template <class Type> constexpr operator Type&() const &  noexcept(_StaticAssertNonInherited<Derived, Type>()) ;
        template <class Type> constexpr operator Type&() const && noexcept(_StaticAssertNonInherited<Derived, Type>()) ;
    };

    template<typename T, typename Any>
    consteval auto _CountMember(auto&&... args)
    {
        if constexpr (!requires{ T{ args... }; })
            return sizeof...(args) - 1;
        else
            return _CountMember<T, Any>(args..., Any{});
    }
} // namespace detail

template<typename T>
consteval auto count_member()
{
    static_assert(
        !std::is_reference<T>::value,
        "====================> Boost.PFR: Attempt to get fields count on a reference. This is not allowed because that could hide an issue and different library users expect different behavior in that case."
    );
    static_assert(
        std::is_aggregate_v<T>, 
        "====================> count_member: Only aggregate types are supported"
    );
    static_assert(
        !std::is_polymorphic<T>::value,
        "====================> count_member: Type must have no virtual function, because otherwise it is not aggregate initializable."
    );
    return detail::_CountMember<T, detail::_Any<T>>();
}

} // namespace skr
// clang-format on