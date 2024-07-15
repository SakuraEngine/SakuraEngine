#pragma once
#include "SkrCore/memory/memory.h"
#include <memory>

// SMART POINTER
namespace skr
{
template <typename T>
using shared_ptr = std::shared_ptr<T>;

template <typename T>
using weak_ptr = std::weak_ptr<T>;

template <typename T>
using unique_ptr = std::unique_ptr<T>;

using std::make_shared;
using std::make_unique;
} // namespace skr
