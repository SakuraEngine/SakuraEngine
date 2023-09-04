#pragma once
#include "SkrBase/config.h"
#include "SkrBase/containers/fwd_container.hpp"
#include "SkrBase/algo/utils.hpp"

namespace skr::container
{
template <typename T>
struct KeyTraits {
    using KeyType       = T;
    using KeyMapperType = MapFwd<T>;
};
} // namespace skr::container