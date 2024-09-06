#pragma once
#include "SkrBase/config.h"

namespace skr::container
{
// TODO. string memory 不能复用 vector memory, 有多种特殊情况:
//  1. size > capacity, 且 capacity 为 0, 代表是常量串
//  2. 默认 string 的 memory 为 SSO
// TODO. 或许最好 string memory 不需要 Base 以最大限度的节约内存
template <typename TS>
struct StringMemoryBase {
    using SizeType = TS;

    // getter
    inline SizeType size() const noexcept { return _size; }
    inline SizeType capacity() const noexcept { return _capacity; }

    // setter
    inline void set_size(SizeType value) noexcept { _size = value; }

    // string literals
    inline bool is_literal() const noexcept { return _size > 0 && _capacity == 0; }

protected:
    void*    _data     = nullptr;
    SizeType _size     = 0;
    SizeType _capacity = 0;
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