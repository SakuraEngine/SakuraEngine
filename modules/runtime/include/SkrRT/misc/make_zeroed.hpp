#pragma once
#include "SkrRT/platform/configure.h"
#include <EASTL/internal/move_help.h>
#include <new> // operator new
#include <string.h> // ::memset

template <typename T, typename... Args>
FORCEINLINE T make_zeroed(Args&&... args)
{
    eastl::aligned_storage_t<sizeof(T)> storage;
    ::memset(&storage, 0, sizeof(storage));
    auto res = new (&storage) T(eastl::forward<Args>(args)...);
    return *res;
}