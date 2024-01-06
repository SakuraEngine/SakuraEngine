#pragma once
#include "SkrRT/goap/atom.hpp"
#include "SkrRT/goap/traits.hpp"
#include "SkrRT/containers/vector.hpp"

namespace skr::goap
{

template <concepts::StaticState T>
struct StaticWorldStateProxy {
    using StateType      = T;
    using IdentifierType = StaticAtomId;
    using ValueStoreType = uint32_t;

protected:
    T _this;
};

template <concepts::StaticState T>
struct StaticCond : public StaticWorldStateProxy<T> {
    using Super          = StaticWorldStateProxy<T>;
    using StateType      = typename Super::StateType;
    using IdentifierType = StaticAtomId;
    using ValueStoreType = uint32_t;

protected:
    const auto& getAtom(const uint32_t idx) const SKR_NOEXCEPT
    {
        const auto offset = idx * sizeof(AtomOpMemory);
        return *reinterpret_cast<const AtomOpMemory*>(reinterpret_cast<const uint8_t*>(&Super::_this) + offset);
    }
    auto& getAtom(const uint32_t idx) SKR_NOEXCEPT
    {
        const auto offset = idx * sizeof(AtomOpMemory);
        return *reinterpret_cast<AtomOpMemory*>(reinterpret_cast<uint8_t*>(&Super::_this) + offset);
    }
    skr::Vector<AtomOperand> operands;
};

template <concepts::StaticState T, StringLiteral Literal = u8"">
struct StaticWorldState : public StaticWorldStateProxy<T> {
    using Super          = StaticWorldStateProxy<T>;
    using StateType      = typename Super::StateType;
    using IdentifierType = typename Super::IdentifierType;
    using ValueStoreType = typename Super::ValueStoreType;

    template <concepts::AtomValue ValueType>
    StaticWorldState& set(const IdentifierType& id, const ValueType& value) SKR_NOEXCEPT
    {
        return set(id.get_index(), static_cast<ValueStoreType>(value));
    }
    template <auto Member>
    requires(concepts::IsAtomMember<Member>)
    StaticWorldState& set(const AtomMemberValueType<Member>& value) SKR_NOEXCEPT
    {
        return set(atom_id<Member>, static_cast<ValueStoreType>(value));
    }

    template <concepts::AtomValue ValueType>
    StaticWorldState& assign(const IdentifierType& id, const ValueType& value) SKR_NOEXCEPT
    {
        auto& atom = getAtom(id.get_index());
        if (atom.exist)
            atom.value = static_cast<ValueStoreType>(value);
        return *this;
    }
    template <auto Member>
    requires(concepts::IsAtomMember<Member>)
    StaticWorldState& assign(const AtomMemberValueType<Member>& value) SKR_NOEXCEPT
    {
        return assign(atom_id<Member>, value);
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

    template <typename F>
    void foreach_variable(F&& func) const
    {
        skr::foreach_field(this->_this, [&](const auto atom, const auto i) {
            if (atom.exist)
                func(i, atom.value);
        });
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
        skr::foreach_field(this->_this, [level](const auto f, const auto i) {
            if (f.exist)
                SKR_LOG_FMT_WITH_LEVEL(level, u8"    {} = {}", f.name, f.value);
        });
    }

protected:
    friend Action<StaticWorldState>;
    
    StaticWorldState& set(const uint32_t& index, const ValueStoreType& value) SKR_NOEXCEPT
    {
        auto& atom = getAtom(index);
        {
            atom.exist = true;
            atom.value = static_cast<ValueStoreType>(value);
        }
        return *this;
    }
    const auto& getAtom(const uint32_t idx) const SKR_NOEXCEPT
    {
        const auto offset = idx * sizeof(AtomMemory);
        return *reinterpret_cast<const AtomMemory*>(reinterpret_cast<const uint8_t*>(&this->_this) + offset);
    }
    auto& getAtom(const uint32_t idx) SKR_NOEXCEPT
    {
        const auto offset = idx * sizeof(AtomMemory);
        return *reinterpret_cast<AtomMemory*>(reinterpret_cast<uint8_t*>(&this->_this) + offset);
    }
};

template <concepts::StaticState T>
using StaticEffect = StaticWorldState<T, u8"Effect">;

} // namespace skr::goap