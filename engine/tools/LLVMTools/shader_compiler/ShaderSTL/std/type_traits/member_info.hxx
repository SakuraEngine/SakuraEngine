#pragma once
#include "builtins.hxx"

namespace cppsl
{
namespace detail
{
template <typename MemberType>
struct MemberInfo;

template <typename T, typename OT>
struct MemberInfo<T(OT::*)> {
    using OwnerType = OT;
    using Type      = T;
};
} // namespace detail

/*
template <typename T>
struct StructInfo {
    inline static constexpr auto Value = std::decay_t<T>();
};
*/

template <auto Member> requires(cppsl::is_member_object_pointer_v<decltype(Member)>)
struct MemberInfo
{
    using PtrType   = decltype(Member);
    using OwnerType = typename detail::MemberInfo<PtrType>::OwnerType;
    using Type      = typename detail::MemberInfo<PtrType>::Type;
};

} // namespace cppsl