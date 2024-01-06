#pragma once
#include "SkrRT/goap/traits/state.hpp"

namespace skr::goap
{
namespace concepts
{
template <auto Member>
inline static constexpr bool IsMemberObject = std::is_member_object_pointer_v<decltype(Member)>;

} // namespace concepts

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

template <concepts::StaticWorldState T>
struct StructInfo {

};

template <auto Member> requires(concepts::IsMemberObject<Member>)
struct MemberInfo {
    using PtrType   = decltype(Member);
    using OwnerType = typename detail::MemberInfo<PtrType>::OwnerType;
    using Type      = typename detail::MemberInfo<PtrType>::Type;
};

} // namespace skr::goap