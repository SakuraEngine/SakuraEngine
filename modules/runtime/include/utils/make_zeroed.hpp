#pragma once
#include "platform/configure.h"
#include <EASTL/internal/move_help.h>
#include <EASTL/type_traits.h>
#include <new> // operator new
#include <cstring>

template <typename T, typename... Args>
FORCEINLINE T make_zeroed(Args&&... args)
{
    eastl::aligned_storage_t<sizeof(T)> storage;
    std::memset(&storage, 0, sizeof(storage));
    auto res = new (&storage) T(eastl::forward<Args>(args)...);
    return *res;
}