#pragma once
#include <EASTL/type_traits.h>
#include <cstring>

template <typename T, typename... Args>
FORCEINLINE T make_zeroed(Args&&... args)
{
    eastl::aligned_storage_t<sizeof(T)> storage;
    std::memset(&storage, 0, sizeof(storage));
    auto res = new (&storage) T(eastl::forward<Args>(args)...);
    return *res;
}