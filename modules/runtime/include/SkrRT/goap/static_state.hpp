#pragma once
#include "SkrRT/goap/traits.hpp"
#include "SkrRT/misc/log.hpp"

namespace skr::goap
{
struct StateBase {
    uint32_t value;
    bool     exist;
};

template <concepts::StaticState T, StringLiteral Literal>
struct State : public StateBase {
    static constexpr const char* Name = Literal.value;
};

static_assert(sizeof(StateBase) == sizeof(State<bool, "__Boolean_state">));
static_assert(sizeof(StateBase) == sizeof(State<EConditionType, "__Enum_state">));

struct StaticStateId {
    template <auto Member> requires(concepts::IsMemberObject<Member>)
    static constexpr StaticStateId Create()
    {
        using OwnerType = MemberInfo<Member>::OwnerType;
        return StaticStateId(0/*TODO*/);
    }
    constexpr StaticStateId(uint32_t index) : index(index) {}
    constexpr uint32_t get_offset() const { return index * sizeof(StateBase); }
    const uint32_t index = 0;
};

template <auto Member> requires(concepts::IsMemberObject<Member>)
inline constexpr StaticStateId id = StaticStateId::Create<Member>();

constexpr auto sz = sizeof(State<bool, "!!!">);

template <StringLiteral Literal>
using BoolState = State<bool, Literal>;

struct StaticWorldState {
    using IdentifierType = OffsetType;
};

} // namespace skr::goap

namespace skr::goap
{
struct TestStates {
    State<bool, "a"> a;
    State<bool, "a"> b;
    int c;
};

constexpr auto fn = count_member<TestStates>();



} // namespace skr::goap