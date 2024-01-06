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
    T _this;
};

} // namespace skr::goap