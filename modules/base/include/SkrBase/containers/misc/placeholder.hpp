#pragma once
#include "SkrBase/config.h"
#include "SkrBase/misc/integer_tools.hpp"
#include "SkrBase/misc/debug.h"

namespace skr::container
{
template <uint64_t Size, uint64_t Align>
struct AlignedStorage {
    alignas(Align) uint8_t storage[Size];
};

template <typename T>
struct Placeholder : AlignedStorage<sizeof(T), alignof(T)> {
};

}; // namespace skr::container