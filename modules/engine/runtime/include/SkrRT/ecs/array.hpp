#pragma once
#include "SkrRT/ecs/llvm/SmallVector.h" // IWYU pragma: export

struct sugoi_array_comp_t : llvm_vecsmall::SmallVectorBase {
    using SmallVectorBase::SmallVectorBase;
};

namespace sugoi
{
template <class T, size_t N>
struct ArrayComponent : public llvm_vecsmall::SmallVector<T, N>
{

};
} // namespace sugoi