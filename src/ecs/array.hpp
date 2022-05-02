#pragma once
#include "SmallVector.h"
#include "ecs/dual.h"
#include "constants.hpp"
struct dual_array_component_t : llvm_vecsmall::SmallVectorBase {
    using SmallVectorBase::SmallVectorBase;
};

namespace dual
{
template <class T, size_t N>
using array_component_T = llvm_vecsmall::SmallVector<T, N / sizeof(T)>;

using link_array_t = array_component_T<dual_entity_t, kLinkComponentSize>;
} // namespace dual