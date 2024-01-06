#pragma once
#include "SkrRT/goap/atom.hpp"
#include "SkrRT/goap/traits.hpp"

namespace skr::goap
{

template <concepts::StaticState T, StringLiteral Literal = u8"">
struct StaticWorldState {
    using StateType      = T;
    using IdentifierType = StaticAtomId;
    using ValueStoreType = uint32_t;

    template <concepts::AtomValue ValueType>
    StaticWorldState& set_variable(const IdentifierType& id, const ValueType& value) SKR_NOEXCEPT
    {
        auto& atom = getAtom(id.get_index());
        {
            atom.exist = true;
            atom.value = static_cast<uint32_t>(value);
        }
        return *this;
    }
    template <auto Member, concepts::AtomValue ValueType>
    StaticWorldState& set_variable(const ValueType& value) SKR_NOEXCEPT
    {
        return set_variable<ValueType>(atom_id<Member>, value);
    }

    template <concepts::AtomValue ValueType>
    StaticWorldState& assign_variable(const IdentifierType& id, const ValueType& value) SKR_NOEXCEPT
    {
        auto& atom = getAtom(id.get_index());
        if (atom.exist)
            atom.value = static_cast<uint32_t>(value);
        return *this;
    }
    template <auto Member, concepts::AtomValue ValueType>
    StaticWorldState& assign_variable(const ValueType& value) SKR_NOEXCEPT
    {
        return assign_variable<ValueType>(atom_id<Member>, value);
    }

    template <concepts::AtomValue ValueType>
    bool get_variable(const uint32_t index, ValueType& value) const SKR_NOEXCEPT
    {
        const auto& atom = getAtom(index);
        if (atom.exist)
            value = static_cast<ValueType>(atom.value);
        return atom.exist;
    }

    template <concepts::AtomValue ValueType>
    bool get_variable(const IdentifierType& id, ValueType& value) const SKR_NOEXCEPT
    {
        const auto index = id.get_index();
        return get_variable<ValueType>(index, value);
    }

    SKR_NOINLINE bool meets_goal(const StaticWorldState& goal) const SKR_NOEXCEPT
    {
        bool fail = false;
        skr::foreach_field(goal._this, [&](const auto f, const auto i) {
            if (f.exist && !fail)
            {
                decltype(f.value) value;
                const bool        found = get_variable(i, value);
                if (!found || Compare<decltype(f.value)>::NotEqual(f.value, value))
                    fail = true;
            }
        });
        return !fail;
    }

    uint64_t distance_to(const StaticWorldState& goal) const SKR_NOEXCEPT
    {
        uint64_t distance = 0;
        skr::foreach_field(goal._this, [&](const auto f, const auto i) {
            if (f.exist)
            {
                decltype(f.value) value;
                const bool        found = get_variable(i, value);
                if (!found || Compare<decltype(f.value)>::NotEqual(f.value, value))
                    distance += 1;
            }
        });
        return distance;
    }

    bool operator==(const StaticWorldState& other) const SKR_NOEXCEPT
    {
        bool fail = false;
        skr::foreach_field(other._this, [&](const auto f, const auto i) {
            if (f.exist && !fail)
            {
                decltype(f.value) value;
                const bool        found = get_variable(i, value);
                if (!found || Compare<decltype(f.value)>::NotEqual(f.value, value))
                    fail = true;
            }
        });
        return !fail;
    }

    void dump(const char8_t* what, int level = SKR_LOG_LEVEL_INFO) const
    {
        SKR_LOG_FMT_WITH_LEVEL(level, u8"{} StaticWorldState: {}", what, Literal.view());
        skr::foreach_field(_this, [level](const auto f, const auto i) {
            if (f.exist)
                SKR_LOG_FMT_WITH_LEVEL(level, u8"    {} = {}", f.name, f.value);
        });
    }

private:
    const auto& getAtom(const uint32_t idx) const SKR_NOEXCEPT
    {
        const auto offset = idx * sizeof(AtomBase);
        return *reinterpret_cast<const AtomBase*>(reinterpret_cast<const uint8_t*>(&_this) + offset);
    }
    auto& getAtom(const uint32_t idx) SKR_NOEXCEPT
    {
        const auto offset = idx * sizeof(AtomBase);
        return *reinterpret_cast<AtomBase*>(reinterpret_cast<uint8_t*>(&_this) + offset);
    }
    T _this;
};

} // namespace skr::goap

namespace skr::goap::test
{
struct TestStates {
    Atom<bool, u8"b"> a;
    Atom<bool, u8"b"> b;
};

constexpr auto fn  = atom_count<TestStates>;
constexpr auto fn2 = atom_count<StaticWorldState<TestStates>>;
static_assert(fn == 2);
static_assert(fn2 == 2);

} // namespace skr::goap::test