#pragma once
#include "SkrBase/misc/hash.hpp"
#include "SkrRT/goap/traits.hpp"
#include "SkrCore/log.hpp"

namespace skr::goap
{
template <bool WithCond>
struct _AtomMemory;
template <>
struct _AtomMemory<false>
{
    uint32_t value = 0;
    bool     exist = false;
};
template <>
struct _AtomMemory<true>
{
    uint32_t value = 0;
    bool     exist = false;
    uint8_t cond = 0;
    uint8_t flag = 0;
};
using AtomMemory = _AtomMemory<false>;
using AtomOpMemory = _AtomMemory<true>;
static_assert(sizeof(AtomMemory) == sizeof(AtomOpMemory));
static_assert(alignof(AtomMemory) == alignof(AtomOpMemory));

struct AtomOperand
{
    uint32_t idx = 0;
    uint32_t value = 0;
    EConditionType condition = EConditionType::Equal;
    EVariableFlag flag = EVariableFlag::Explicit;
};

template <concepts::AtomValue T, StringLiteral Literal>
struct Atom : public AtomMemory {
    using ValueType = T;

    static constexpr skr::StringView name = Literal.view();
};

template <StringLiteral Literal>
using BoolAtom = Atom<bool, Literal>;

namespace concepts
{
template <typename T>
inline constexpr bool IsAtom = false;

template <concepts::AtomValue T, StringLiteral Literal>
inline constexpr bool IsAtom<Atom<T, Literal>> = true;

template <typename T>
concept AtomType = IsAtom<T>;

struct AtomCheck {
    // when check failed, output both StateType and FieldType
    template <typename StateType, typename FieldType>
    inline static constexpr bool CheckAtom = IsAtom<FieldType>;

    template <class Type, typename FieldType>
    static constexpr bool Check() noexcept 
    { 
        static_assert(
            CheckAtom<Type, FieldType>, 
            "====================> AtomCheck: all WorldState field must be atom." 
        );
        static_assert(
            sizeof(FieldType) == sizeof(AtomMemory), 
            "====================> AtomCheck: field atom size mismatch." 
        );
        return true; 
    }
};

template <auto Member>
inline constexpr bool IsAtomMember = IsAtom<typename MemberInfo<Member>::Type> &&
                                     IsStaticState<typename MemberInfo<Member>::OwnerType>;

} // namespace concepts

template <concepts::AtomType T>
using AtomValueType = typename T::ValueType;

template <auto Member> requires(concepts::IsAtomMember<Member>) 
using AtomMemberValueType = AtomValueType<typename MemberInfo<Member>::Type>;

static_assert(sizeof(AtomMemory) == sizeof(Atom<bool, u8"__Boolean_state">));
static_assert(sizeof(AtomMemory) == sizeof(Atom<EConditionType, u8"__Enum_state">));

struct StaticAtomId {
    constexpr StaticAtomId(uint32_t offset)
        : offset(offset)
    {
    }
    const uint32_t get_index() const { return offset / sizeof(AtomMemory); }
    const bool operator==(const StaticAtomId& other) const { return offset == other.offset; }
    const uint32_t offset = 0;
};

template <auto Member> requires(concepts::IsAtomMember<Member>)
inline static const StaticAtomId atom_id = StaticAtomId(MemberInfo<Member>::Offset);

template <concepts::StaticState T>
inline constexpr uint32_t atom_count = skr::count_member<T, concepts::AtomCheck>();

template <concepts::StaticState T, StringLiteral Literal>
inline constexpr uint32_t atom_count<StaticWorldState<T, Literal>> = skr::count_member<T, concepts::AtomCheck>();

} // namespace skr::goap

namespace skr {
template <>
struct Hash<goap::StaticAtomId> {
    inline size_t operator()(const goap::StaticAtomId& id) const noexcept
    {
        return id.offset;
    }
};
} // namespace skr
