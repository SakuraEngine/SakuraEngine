#pragma once
#include "SkrRuntime/goap/traits/state.hpp"

namespace skr::goap
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

template <auto Member> requires(std::is_member_object_pointer_v<decltype(Member)>)
struct MemberInfo {
    using PtrType   = decltype(Member);
    using OwnerType = typename detail::MemberInfo<PtrType>::OwnerType;
    using Type      = typename detail::MemberInfo<PtrType>::Type;

    inline static const auto Offset = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&(static_cast<OwnerType*>(nullptr)->*Member)));
};

} // namespace skr::goap