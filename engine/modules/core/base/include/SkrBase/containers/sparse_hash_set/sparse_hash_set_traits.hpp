#pragma once
#include "SkrBase/algo/utils.hpp"
#include "SkrBase/misc/hash.hpp"

namespace skr::container
{
template <typename T>
struct HashTraits
{
    using HashType   = skr_hash;
    using HasherType = Hash<T>;
};
} // namespace skr::container