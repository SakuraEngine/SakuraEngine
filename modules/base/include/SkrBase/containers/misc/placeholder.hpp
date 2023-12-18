#pragma once
#include "SkrBase/config.h"
#include "SkrBase/misc/integer_tools.hpp"
#include "SkrBase/misc/debug.h"

namespace skr::container
{
template <uint64_t Size, uint64_t Align>
struct AlignedStorage {
    inline void*       data() noexcept { return reinterpret_cast<void*>(_storage); }
    inline const void* data() const noexcept { return reinterpret_cast<const void*>(_storage); }

private:
    alignas(Align) uint8_t _storage[Size];
};

template <typename T, uint64_t N = 1>
struct Placeholder : AlignedStorage<sizeof(T) * N, alignof(T)> {
    using Super = AlignedStorage<sizeof(T) * N, alignof(T)>;

    inline T* data_typed() noexcept
    {
        return reinterpret_cast<T*>(data());
    }
    inline const T* data_typed() const noexcept
    {
        return reinterpret_cast<const T*>(data());
    }
};

}; // namespace skr::container