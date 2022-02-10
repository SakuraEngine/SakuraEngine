#pragma once
#include "platform/configure.h"

namespace sakura
{
namespace math
{

template <typename Storage, uint32_t FractionBits>
struct fixed {
private:
    Storage storage;
};
using fixed64 = fixed<int64_t, 32u>;
using fixed32 = fixed<int32_t, 16u>;

} // namespace math
} // namespace sakura