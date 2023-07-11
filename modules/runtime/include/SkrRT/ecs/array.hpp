#pragma once
#include "SkrRT/ecs/SmallVector.h"
#include "SkrRT/ecs/dual.h"
#include "SkrRT/ecs/constants.hpp"

struct dual_array_comp_t : llvm_vecsmall::SmallVectorBase {
    using SmallVectorBase::SmallVectorBase;
};

namespace dual
{
template <class T, size_t N>
struct array_comp_T : public llvm_vecsmall::SmallVector<T, N>
{

};
} // namespace dual