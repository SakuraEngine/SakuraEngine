#pragma once
#include "SkrRT/goap/static/proxy.hpp"
#include "SkrContainers/vector.hpp"

namespace skr::goap
{

template <concepts::StaticState T>
struct StaticCond : public StaticWorldStateProxy<T> {
    using Super          = StaticWorldStateProxy<T>;
    using StateType      = typename Super::StateType;
    using IdentifierType = StaticAtomId;
    using ValueStoreType = uint32_t;

    void add(const IdentifierType& id, EVariableFlag flag,
             const ValueStoreType& value, EConditionType cond) SKR_NOEXCEPT
    {
        auto& atom = getAtom(id.get_index());
        atom.exist = true;
        atom.value = value;
        atom.flag  = static_cast<uint8_t>(flag);
        atom.cond  = static_cast<uint8_t>(cond);
    }

    template <typename F>
    bool foreachOperand(F&& func) const
    {
        return foreachAtomOp([&](const auto i, const auto& atom) {
            if (atom.exist)
                return func(i, 
                    static_cast<EVariableFlag>(atom.flag), 
                    atom.value, 
                    static_cast<EConditionType>(atom.cond)
                );
            return true;
        });
    }

protected:
    const auto& getAtom(const uint32_t idx) const SKR_NOEXCEPT
    {
        const auto offset = idx * sizeof(AtomOpMemory);
        return *reinterpret_cast<const AtomOpMemory*>(reinterpret_cast<const uint8_t*>(&this->_this) + offset);
    }
    auto& getAtom(const uint32_t idx) SKR_NOEXCEPT
    {
        const auto offset = idx * sizeof(AtomOpMemory);
        return *reinterpret_cast<AtomOpMemory*>(reinterpret_cast<uint8_t*>(&this->_this) + offset);
    }
    template <typename F>
    bool foreachAtomOp(F&& func) const
    {
#ifndef SKR_GOAP_ERASE_MORE_TYPE
        bool fail = false;
        skr::foreach_field(this->_this, [&](const auto& atom, const auto i) {
            if (!fail)
                fail = !func(i, (const AtomOpMemory&)atom);
        });
        return !fail;
#else
        auto pStart = reinterpret_cast<const AtomOpMemory*>(&this->_this);
        auto pEnd   = pStart + (sizeof(this->_this) / sizeof(AtomOpMemory));
        for (auto p = pStart; p < pEnd; p += 1)
        {
            const auto& atom = *reinterpret_cast<const AtomOpMemory*>(p);
            if (bool fail = !func(p - pStart, atom))
                return false;
        }
        return true;
#endif
    }
    skr::Vector<AtomOperand> extra_operands;
};

} // namespace skr::goap