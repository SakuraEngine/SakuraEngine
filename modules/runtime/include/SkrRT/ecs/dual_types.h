#pragma once
#include "dual_config.h"
#include "misc/types.h"

#if defined(__cplusplus)
extern "C" {
#endif

constexpr uint32_t dead = 2 + (1 << 29);

// objects
#define DUAL_DECLARE(name) typedef struct dual_##name dual_##name
DUAL_DECLARE(context_t);
DUAL_DECLARE(storage_t);
DUAL_DECLARE(group_t);
DUAL_DECLARE(chunk_t);
DUAL_DECLARE(query_t);
DUAL_DECLARE(storage_delta_t);
#undef DUAL_DECLARE

typedef TIndex dual_type_index_t;
typedef skr_guid_t dual_guid_t;

#if defined(__cplusplus)
}
#endif

#if defined(__cplusplus)
#include <EASTL/numeric_limits.h>

namespace dual
{
static constexpr size_t kFastBinSize = 64 * 1024;
static constexpr size_t kSmallBinThreshold = 8;
static constexpr size_t kSmallBinSize = 1024;
static constexpr size_t kLargeBinSize = 1024 * 1024;

static constexpr size_t kFastBinCapacity = 800;
static constexpr size_t kSmallBinCapacity = 200;
static constexpr size_t kLargeBinCapacity = 80;
static constexpr SIndex kInvalidSIndex = eastl::numeric_limits<SIndex>::max();
static constexpr TIndex kInvalidTypeIndex = eastl::numeric_limits<TIndex>::max();

static constexpr size_t kGroupBlockSize = 128 * 4;
static constexpr size_t kGroupBlockCount = 256;
static constexpr size_t kStorageArenaSize = 128 * 128;
static constexpr size_t kLinkComponentSize = 8;

enum pool_type_t
{
    PT_small,
    PT_default,
    PT_large
};

template <class T, size_t N>
struct array_comp_T;
using link_array_t = array_comp_T<dual_entity_t, kLinkComponentSize>;
}
#endif