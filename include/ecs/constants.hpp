#pragma once

#include <limits>
#include "ecs/dual_config.h"
namespace dual
{
static constexpr size_t kFastBinSize = 64 * 1024;
static constexpr size_t kSmallBinThreshold = 8;
static constexpr size_t kSmallBinSize = 1024;
static constexpr size_t kLargeBinSize = 1024 * 1024;

static constexpr size_t kFastBinCapacity = 800;
static constexpr size_t kSmallBinCapacity = 200;
static constexpr size_t kLargeBinCapacity = 80;
static constexpr SIndex kInvalidSIndex = std::numeric_limits<SIndex>::max();
static constexpr TIndex kInvalidTypeIndex = std::numeric_limits<TIndex>::max();

static constexpr size_t kGroupBlockSize = 128 * 2;
static constexpr size_t kGroupBlockCount = 256;
static constexpr size_t kStorageArenaSize = 128 * 128;
static constexpr size_t kLinkComponentSize = 8;

enum pool_type_t
{
    PT_small,
    PT_default,
    PT_large
};
} // namespace dual