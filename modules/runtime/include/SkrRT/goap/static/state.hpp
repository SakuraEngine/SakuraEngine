#pragma once
#include "SkrRT/goap/static/proxy.hpp"
#include "SkrRT/goap/static/cond.hpp"

namespace skr::goap
{

template <concepts::StaticState T, StringLiteral Literal = u8"">
struct StaticWorldState : public StaticWorldStateProxy<T> {
    using Super          = StaticWorldStateProxy<T>;
    using StateType      = typename Super::StateType;
    using IdentifierType = typename Super::IdentifierType;
    using ValueStoreType = typename Super::ValueStoreType;
    using CondType = StaticCond<T>;

    template <typename ValueType>
    StaticWorldState& set(const IdentifierType& id, const ValueType& value) SKR_NOEXCEPT
    {
        return set(id.get_index(), static_cast<ValueStoreType>(value));
    }
    template <auto Member, typename ValueType>
    requires(concepts::IsAtomMember<Member>)
    StaticWorldState& set(const ValueType& value) SKR_NOEXCEPT
    {
        return set(atom_id<Member>, static_cast<ValueStoreType>(value));
    }

    template <typename ValueType>
    StaticWorldState& assign(const IdentifierType& id, const ValueType& value) SKR_NOEXCEPT
    {
        auto& atom = getAtom(id.get_index());
        if (atom.exist)
            atom.value = static_cast<ValueStoreType>(value);
        return *this;
    }
    template <auto Member, typename ValueType>
    requires(concepts::IsAtomMember<Member>)
    StaticWorldState& assign(const ValueType& value) SKR_NOEXCEPT
    {
        return assign(atom_id<Member>, static_cast<ValueStoreType>(value));
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
        goal.foreachAtomMemory([&](const auto i, const auto& atom) {
            if (atom.exist && !fail)
            {
                decltype(atom.value) value;
                const bool        found = get_variable(i, value);
                if (!found || Compare<decltype(atom.value)>::NotEqual(atom.value, value))
                    fail = true;
            }
            return true;
        });
        return !fail;
    }

    uint64_t distance_to(const StaticWorldState& goal) const SKR_NOEXCEPT
    {
        uint64_t distance = 0;
        goal.foreachAtomMemory([&](const auto i, const auto& atom) {
            if (atom.exist)
            {
                decltype(atom.value) value;
                const bool        found = get_variable(i, value);
                if (!found || Compare<decltype(atom.value)>::NotEqual(atom.value, value))
                    distance += 1;
            }
            return true;
        });
        return distance;
    }

    bool operator==(const StaticWorldState& other) const SKR_NOEXCEPT
    {
        bool fail = false;
        other.foreachAtomMemory([&](const auto i, const auto& atom) {
            if (atom.exist && !fail)
            {
                decltype(atom.value) value;
                const bool        found = get_variable(i, value);
                if (!found || Compare<decltype(atom.value)>::NotEqual(atom.value, value))
                    fail = true;
            }
            return true;
        });
        return !fail;
    }

    void dump(const char8_t* what, int level = SKR_LOG_LEVEL_INFO) const
    {
        SKR_LOG_FMT_WITH_LEVEL(level, u8"{} StaticWorldState: {}", what, Literal.view());
        skr::foreach_field(this->_this, [level](const auto f, const auto i) {
            if (f.exist)
                SKR_LOG_FMT_WITH_LEVEL(level, u8"    {} = {}", f.name, f.value);
            return true;
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
    template <typename F>
    bool foreachAtomValue(F&& func) const
    {
#ifndef SKR_GOAP_ERASE_MORE_TYPE
        bool fail = false;
        skr::foreach_field(this->_this, [&](const auto& atom, const auto i) {
            if (atom.exist && !fail)
                fail = !func(i, atom.value);
        });
        return !fail;
#else
        auto pStart = reinterpret_cast<const AtomMemory*>(&this->_this);
        auto pEnd   = pStart + (sizeof(this->_this) / sizeof(AtomMemory));
        for (auto p = pStart; p < pEnd; p += 1)
        {
            const auto& atom = *reinterpret_cast<const AtomMemory*>(p);
            bool fail = false;
            if (atom.exist)
                fail = !func(p - pStart, atom.value);
            if (fail)
                return false;
        }
        return true;
#endif
    }
    template <typename F>
    bool foreachAtomMemory(F&& func) const
    {
#ifndef SKR_GOAP_ERASE_MORE_TYPE
        bool fail = false;
        skr::foreach_field(this->_this, [&](const auto& atom, const auto i) {
            if (!fail)
                fail = !func(i, atom);
        });
        return !fail;
#else
        auto pStart = reinterpret_cast<const AtomMemory*>(&this->_this);
        auto pEnd   = pStart + (sizeof(this->_this) / sizeof(AtomMemory));
        for (auto p = pStart; p < pEnd; p += 1)
        {
            const auto& atom = *reinterpret_cast<const AtomMemory*>(p);
            if (bool fail = !func(p - pStart, atom))
                return false;
        }
        return true;
#endif
    }
};

} // namespace skr::goap