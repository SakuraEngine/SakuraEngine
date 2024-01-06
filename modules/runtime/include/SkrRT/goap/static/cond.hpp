#pragma once
#include "SkrRT/goap/static/proxy.hpp"
#include "SkrRT/containers/vector.hpp"

namespace skr::goap
{

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
    skr::Vector<AtomOperand> extra_operands;
};

} // namespace skr::goap