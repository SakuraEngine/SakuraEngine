#pragma once
#include "SkrBase/algo/utils.hpp"
#include "SkrBase/misc/hash.hpp"

// TODO. HashTraits 与 KeyTraits 应当解绑，指定 key 的对象理应实现特殊的 Hasher
namespace skr::container
{
template <typename T>
struct KeyTraits {
    using KeyType         = T;
    using KeyMapperType   = MapFwd<T>;
    using KeyComparerType = Equal<T>;
};
} // namespace skr::container

namespace skr::container
{
template <typename T>
struct HashTraits {
    using HashType   = size_t;
    using HasherType = Hash<T>;
};
} // namespace skr::container