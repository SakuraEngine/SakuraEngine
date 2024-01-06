#pragma once
#include "SkrRT/goap/traits.hpp"
#include "SkrRT/misc/log.hpp"

namespace skr::goap
{
struct AtomBase {
    uint32_t value;
    bool     exist;
};

template <concepts::AtomicValue T, StringLiteral Literal>
struct Atom : public AtomBase {
    static constexpr const char* Name = Literal.value;
};

template <StringLiteral Literal>
using BoolState = Atom<bool, Literal>;

namespace concepts
{
template <typename T>
inline constexpr bool IsAtom = false;

template <concepts::AtomicValue T, StringLiteral Literal>
inline constexpr bool IsAtom<Atom<T, Literal>> = true;
} // namespace concepts

static_assert(sizeof(AtomBase) == sizeof(Atom<bool, "__Boolean_state">));
static_assert(sizeof(AtomBase) == sizeof(Atom<EConditionType, "__Enum_state">));

struct StaticAtomId {
    template <auto Member> requires(concepts::IsMemberObject<Member>)
    static constexpr StaticAtomId Create()
    {
        // using OwnerType = typename MemberInfo<Member>::OwnerType;
        return StaticAtomId(0 /*TODO*/);
    }
    constexpr StaticAtomId(uint32_t index)
        : index(index)
    {
    }
    constexpr uint32_t get_offset() const { return index * sizeof(AtomBase); }
    const uint32_t     index = 0;
};

template <auto Member> requires(concepts::IsMemberObject<Member>)
inline constexpr StaticAtomId atom_id = StaticAtomId::Create<Member>();
} // namespace skr::goap