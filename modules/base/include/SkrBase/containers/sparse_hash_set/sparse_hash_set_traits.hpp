#pragma once
#include "SkrBase/algo/utils.hpp"
#include "SkrBase/misc/hash.hpp"

namespace skr::container
{
template <typename T>
struct HashTraits {
    using HashType   = size_t;
    using HasherType = Hash<T>;
};
} // namespace skr::container