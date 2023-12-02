#pragma once
#include "SkrRT/config.h"  // IWYU pragma: keep
#include "SkrMemory/smart_ptr/smart_ptr.hpp"

// SMART POINTER
namespace skr
{
template <typename T>
using shared_ptr = skr::container::shared_ptr<T>;

template <typename T>
using weak_ptr = skr::container::weak_ptr<T>;

using skr::container::make_shared;
}
