#pragma once
#include "SkrBase/math.hpp" // IWYU pragma: export
#include "SkrRuntime/ecs/component.hpp" // IWYU pragma: export
#include "SkrContainersDef/array.hpp"

namespace skr
{
using namespace skr;
using uint32 = ::uint32_t;
using uint32_t = ::uint32_t;
using uint64 = ::uint64_t;
using uint64_t = ::uint64_t;
using AddressType = uint32_t;

template <class T, size_t N>
using gpu_array = skr::Array<T, N>;

} // namespace skr