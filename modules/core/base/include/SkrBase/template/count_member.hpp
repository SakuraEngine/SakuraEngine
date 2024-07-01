#pragma once
#include "SkrBase/config.h"
#include <type_traits>

// clang-format off
namespace skr {
namespace detail {
    template <class T>
    inline constexpr bool always_false_v = false;

    template <class Type, class FieldType>
    constexpr bool _StaticAssertNonInherited() noexcept {
        static_assert(
            !std::is_base_of<FieldType, Type>::value,
            "====================> count_member: Inherited types are not supported."
        );
        return true;
    }

    struct _EmptyCheck {
    template <class Type, class FieldType>
        static constexpr bool Check() noexcept { return true; }
    };

    template <typename Type, typename CheckTrait>
    struct _Any 
    {
        template <class FieldType> constexpr operator FieldType&() const &  noexcept(CheckTrait::template Check<Type, FieldType>() && _StaticAssertNonInherited<Type, FieldType>()) ;
        template <class FieldType> constexpr operator FieldType&() const && noexcept(CheckTrait::template Check<Type, FieldType>() && _StaticAssertNonInherited<Type, FieldType>()) ;
    };

    template<typename T, typename Any>
    consteval auto _CountMemberNonFlatten(auto&&... args)
    {
        if constexpr (!requires{ T{ args... }; })
            return sizeof...(args) - 1;
        else
            return _CountMemberNonFlatten<T, Any>(args..., Any{});
    }

    template<typename T, typename Any>
    constexpr size_t _CountMember();
} // namespace detail

template<typename T, typename CheckTrait = detail::_EmptyCheck>
consteval auto count_member()
{
    static_assert(
        !std::is_reference<T>::value,
        "====================> count_member: Attempt to get fields count on a reference. This is not allowed because that could hide an issue and different library users expect different behavior in that case."
    );
    static_assert(
        std::is_aggregate_v<T>, 
        "====================> count_member: Only aggregate types are supported"
    );
    static_assert(
        !std::is_polymorphic<T>::value,
        "====================> count_member: Type must have no virtual function, because otherwise it is not aggregate initializable."
    );
    return detail::_CountMember<T, detail::_Any<T, CheckTrait>>();
}

} // namespace skr

namespace skr::detail
{
template<typename T, typename Any>
constexpr size_t _CountMember() {
static_assert(std::is_aggregate_v<T>, "Only aggregate types are supported");
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){ 
if constexpr(!requires{T{Any{},Any{},Any{},Any{}};}){ 
if constexpr(!requires{T{Any{},Any{}};}){ 
return 1;
}else{
if constexpr(!requires{T{Any{},Any{},Any{}};}){ 
return 2;
}else{
return 3;
}
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{}};}){ 
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{}};}){
return 4;
}else{
return 5;
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 6;
}else{
return 7;
}
}
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 8;
}else{
return 9;
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 10;
}else{
return 11;
}
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 12;
}else{
return 13;
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 14;
}else{
return 15;
}
}
}
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 16;
}else{
return 17;
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 18;
}else{
return 19;
}
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 20;
}else{
return 21;
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 22;
}else{
return 23;
}
}
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 24;
}else{
return 25;
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 26;
}else{
return 27;
}
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 28;
}else{
return 29;
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 30;
}else{
return 31;
}
}
}
}
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 32;
}else{
return 33;
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 34;
}else{
return 35;
}
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 36;
}else{
return 37;
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 38;
}else{
return 39;
}
}
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 40;
}else{
return 41;
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 42;
}else{
return 43;
}
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 44;
}else{
return 45;
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 46;
}else{
return 47;
}
}
}
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 48;
}else{
return 49;
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 50;
}else{
return 51;
}
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 52;
}else{
return 53;
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 54;
}else{
return 55;
}
}
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 56;
}else{
return 57;
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 58;
}else{
return 59;
}
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 60;
}else{
return 61;
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 62;
}else{
return 63;
}
}
}
}
}
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 64;
}else{
return 65;
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 66;
}else{
return 67;
}
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 68;
}else{
return 69;
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 70;
}else{
return 71;
}
}
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 72;
}else{
return 73;
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 74;
}else{
return 75;
}
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 76;
}else{
return 77;
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 78;
}else{
return 79;
}
}
}
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 80;
}else{
return 81;
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 82;
}else{
return 83;
}
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 84;
}else{
return 85;
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 86;
}else{
return 87;
}
}
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 88;
}else{
return 89;
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 90;
}else{
return 91;
}
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 92;
}else{
return 93;
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 94;
}else{
return 95;
}
}
}
}
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){     
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 96;
}else{
return 97;
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 98;
}else{
return 99;
}
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 100;
}else{
return 101;
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 102;
}else{
return 103;
}
}
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 104;
}else{
return 105;
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 106;
}else{
return 107;
}
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 108;
}else{
return 109;
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){        
return 110;
}else{
return 111;
}
}
}
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){  
return 112;
}else{
return 113;
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 114;
}else{
return 115;
}
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 116;
}else{
return 117;
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 118;
}else{
return 119;
}
}
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 120;
}else{
return 121;
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 122;
}else{
return 123;
}
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 124;
}else{
return 125;
}
}else{
if constexpr(!requires{T{Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{},Any{}};}){
return 126;
}else{
    return _CountMemberNonFlatten<T, Any>();
}
}
}
}
}
}
}
}
} // namespace skr::detail
// clang-format on