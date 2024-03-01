#pragma once
#include "SkrBase/config.h"
#include <type_traits> // std::aligned_storage_t
#include <new> // operator new
#include <string.h> // ::memset

template <typename T, typename... Args>
SKR_FORCEINLINE T make_zeroed(Args&&... args)
{
    std::aligned_storage_t<sizeof(T)> storage;
    ::memset(&storage, 0, sizeof(storage));
    auto res = new (&storage) T(std::forward<Args>(args)...);
    return *res;
}