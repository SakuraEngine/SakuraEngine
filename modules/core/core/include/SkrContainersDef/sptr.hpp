#pragma once
#include "detail/sptr.hpp"      // IWYU pragma: export
#include "detail/sweak_ptr.hpp" // IWYU pragma: keep

namespace skr
{
template <typename T, bool EmbedRC>
struct SPtrHelper;
template <typename T>
using SPtr = SPtrHelper<T, true>;
template <typename T>
using SObjectPtr = SPtrHelper<T, false>;
} // namespace skr