#pragma once
#include "SkrRT/config.h"
#include "SkrRT/serde/binary/writer_fwd.h"
#include "SkrRT/serde/binary/reader_fwd.h"
// TODO: REMOVE EASTL
#include <EASTL/bitset.h>
#include <EASTL/fixed_vector.h>
#include <EASTL/fixed_set.h>
#include <EASTL/fixed_string.h>
#include <EASTL/fixed_map.h>
#include <EASTL/shared_ptr.h>

// SMART POINTER
namespace skr
{
template <typename T>
using shared_ptr = eastl::shared_ptr<T>;

template <typename T>
using weak_ptr = eastl::weak_ptr<T>;

template <typename T>
using unique_ptr = eastl::unique_ptr<T>;

using eastl::make_shared;
using eastl::make_unique;
} // namespace skr

// FIXED CONTAINERS
namespace skr
{
template <typename CharT, size_t N>
using FixedString = eastl::fixed_string<CharT, N>;
} // namespace skr
// FIXED CONTAINERS
