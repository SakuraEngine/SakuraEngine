#pragma once
#include "sugoi_config.h"
#include "SkrBase/types.h"

#if defined(__cplusplus)
extern "C" {
#endif

[[maybe_unused]] constexpr uint32_t dead = 2 + (1 << 29);

// objects
#define SUGOI_DECLARE(name) typedef struct sugoi_##name sugoi_##name
SUGOI_DECLARE(context_t);
SUGOI_DECLARE(storage_t);
SUGOI_DECLARE(group_t);
SUGOI_DECLARE(chunk_t);
SUGOI_DECLARE(query_t);
SUGOI_DECLARE(storage_delta_t);
#undef SUGOI_DECLARE

typedef TIndex     sugoi_type_index_t;
typedef skr_guid_t sugoi_guid_t;

#if defined(__cplusplus)
}
#endif

#if defined(__cplusplus)
    #include <limits>

namespace skr::archive
{
    struct BinaryReader;
    struct BinaryWriter;
    struct JsonReader;
    struct JsonWriter;
}

namespace sugoi
{
[[maybe_unused]] static constexpr size_t kFastBinSize       = 64 * 1024;
[[maybe_unused]] static constexpr size_t kSmallBinThreshold = 8;
[[maybe_unused]] static constexpr size_t kSmallBinSize      = 1024;
[[maybe_unused]] static constexpr size_t kLargeBinSize      = 1024 * 1024;

[[maybe_unused]] static constexpr size_t kFastBinCapacity  = 800;
[[maybe_unused]] static constexpr size_t kSmallBinCapacity = 200;
[[maybe_unused]] static constexpr size_t kLargeBinCapacity = 80;
[[maybe_unused]] static constexpr SIndex kInvalidSIndex    = std::numeric_limits<SIndex>::max();
[[maybe_unused]] static constexpr TIndex kInvalidTypeIndex = std::numeric_limits<TIndex>::max();

[[maybe_unused]] static constexpr size_t kGroupBlockSize    = 128 * 4;
[[maybe_unused]] static constexpr size_t kGroupBlockCount   = 256;
[[maybe_unused]] static constexpr size_t kStorageArenaSize  = 128 * 128;
[[maybe_unused]] static constexpr size_t kLinkComponentSize = 8;

enum pool_type_t
{
    PT_small,
    PT_default,
    PT_large
};

template <class T, size_t N>
struct ArrayComponent;
using link_array_t = ArrayComponent<sugoi_entity_t, kLinkComponentSize>;
} // namespace sugoi
#endif