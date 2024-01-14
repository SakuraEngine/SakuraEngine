#pragma once
#include "SkrRT/goap/atom.hpp"

namespace skr::goap
{

template <concepts::StaticState T>
struct StaticWorldStateProxy {
    using StateType      = T;
    using IdentifierType = StaticAtomId;
    using ValueStoreType = uint32_t;

protected:
    static constexpr uint32_t AtomCount = atom_count<T>;
    static_assert(AtomCount || !AtomCount, "Invalid StaticWorldState! Failed to pass concepts::AtomCheck.");
    T _this;
};

} // namespace skr::goap