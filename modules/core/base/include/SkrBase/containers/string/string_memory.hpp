#pragma once
#include "SkrBase/config.h"

namespace skr::container
{
// TODO. string memory 不能复用 vector memory, 有多种特殊情况:
//  1. size > capacity, 且 capacity 为 0, 代表是常量串
//  2. 默认 string 的 memory 为 SSO
struct StringMemoryBase {
};

// SSO string memory
struct StringMemory {
};

// COW string memory
struct COWStringMemory {
};

// serde string memory
struct SerdeStringMemory {
};

} // namespace skr::container