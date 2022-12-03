#pragma once
#include "ecs/SmallVector.h"
#include "ecs/dual.h"
#include "ecs/constants.hpp"

struct dual_array_component_t : llvm_vecsmall::SmallVectorBase {
    using SmallVectorBase::SmallVectorBase;
};

namespace dual
{
template <class T, size_t N>
struct array_component_T : public llvm_vecsmall::SmallVector<T, N>
{

};
} // namespace dual