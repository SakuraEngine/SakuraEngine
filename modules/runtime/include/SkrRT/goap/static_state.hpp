#pragma once
#include "SkrRT/goap/atom.hpp"

namespace skr::goap
{

template <concepts::StaticState T, StringLiteral Literal = "">
struct StaticWorldState {
    using IdentifierType = OffsetType;
};

} // namespace skr::goap

namespace skr::goap
{
struct TestStates {
    Atom<bool, "a"> a;
    Atom<bool, "b"> b;
};

template <typename FieldType>
concept StateField = concepts::IsAtom<FieldType>;
struct FieldCheck {
    template <class Type, StateField FieldType>
    static constexpr bool Check() noexcept { return true; }
};

constexpr auto fn = count_member<TestStates, FieldCheck>();
static_assert(fn == 2);

} // namespace skr::goap