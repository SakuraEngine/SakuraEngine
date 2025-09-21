#pragma once
#include "SkrBase/atomic/atomic_mutex.hpp"
#include "SkrRuntime/sugoi/sugoi_types.h" // IWYU pragma: keep
#include "SkrRuntime/sugoi/archetype.hpp" // IWYU pragma: keep
#include "SkrRuntime/sugoi/type_index.hpp" // IWYU pragma: keep

struct sugoi_chunk_t;
struct sugoi_chunk_view_t;
struct sugoi_group_t;

namespace sugoi
{
struct archetype_t;
struct type_index_t;
typedef uint16_t sugoi_lck_atomic_t;

using slice_lock_t = skr::shared_atomic_mutex;
static_assert(sizeof(slice_lock_t) == sizeof(uint32_t), 
    "Now we use 32-bit storage for a single slice lock, because sizeof(lck) + sizeof(data) can fit into 64-bit"
);

struct slice_data_t
{
    sugoi_timestamp_t timestamp;
private:
    friend struct ::sugoi_chunk_t;
    mutable slice_lock_t lck;
};
static_assert(sizeof(slice_data_t) == sizeof(uint64_t), 
    "Per slice data is 8 bytes, which is the same size as a single 64-bit integer"
);
}

struct sugoi_chunk_t {
    sugoi_chunk_t(sugoi::pool_type_t pt)
        : pt(pt)
    {
    }
    uint32_t index;
    sugoi::archetype_t* structure = nullptr;
    sugoi_group_t* group = nullptr;
    EIndex count = 0;
    sugoi::pool_type_t pt;

    static sugoi_chunk_t* create(sugoi::pool_type_t poolType);
    static void destroy(sugoi_chunk_t* chunk);
    void init(sugoi::archetype_t* structure);

    struct RWSlice { char * start, * end; };
    struct RSlice { char * const start, * const end; };
    RWSlice x_lock(const sugoi_type_index_t& type, const sugoi_chunk_view_t& view);
    RSlice s_lock(const sugoi_type_index_t& type, const sugoi_chunk_view_t& view) const;
    void x_unlock(const sugoi_type_index_t& type, const sugoi_chunk_view_t& view);
    void s_unlock(const sugoi_type_index_t& type, const sugoi_chunk_view_t& view) const;

    SUGOI_FORCEINLINE EIndex get_capacity()
    {
        return structure->chunkCapacity[pt];
    }

    SUGOI_FORCEINLINE const sugoi_entity_t* get_entities() const
    {
        return (const sugoi_entity_t*)data();
    }

    SUGOI_FORCEINLINE sugoi_timestamp_t get_timestamp_at(uint32_t idx) const
    {
        const auto pData = getSliceData();
        return pData[idx].timestamp;
    }

    SUGOI_FORCEINLINE sugoi_timestamp_t get_timestamp(sugoi_type_index_t type) const
    {
        const auto idx = structure->index(type);
        return get_timestamp_at(idx);
    }

    SUGOI_FORCEINLINE sugoi_timestamp_t set_timestamp_at(uint32_t at, sugoi_timestamp_t ts)
    {
        const auto pData = getSliceData();
        pData[at].timestamp = ts;
        return ts;
    }

    SUGOI_FORCEINLINE sugoi_timestamp_t set_timestamp(sugoi_type_index_t type, sugoi_timestamp_t ts)
    {
        const auto idx = structure->index(type);
        return set_timestamp_at(idx, ts);
    }

    SUGOI_FORCEINLINE RWSlice get_unsafe(const sugoi_type_index_t& type, const sugoi_chunk_view_t& view)
    {
        EIndex offset = 0;
        const auto id = structure->index(type);
        if (!sugoi::type_index_t(type).is_chunk())
            offset = structure->sizes[id] * view.start;
        return { 
            data() + offset + structure->offsets[pt][id],
            data() + offset + structure->offsets[pt][id] + structure->sizes[id] * view.count
        };
    }

    SUGOI_FORCEINLINE RSlice get_unsafe(const sugoi_type_index_t& type, const sugoi_chunk_view_t& view) const
    {
        EIndex offset = 0;
        const auto id = structure->index(type);
        if (!sugoi::type_index_t(type).is_chunk())
            offset = structure->sizes[id] * view.start;
        return { 
            data() + offset + structure->offsets[pt][id],
            data() + offset + structure->offsets[pt][id] + structure->sizes[id] * view.count
        };
    }
    
    // TODO: HIDE THESE
    char* data() { return (char*)(this + 1); }
    char* data() const { return (char*)(this + 1); }
private:
    SUGOI_FORCEINLINE sugoi::slice_data_t const* getSliceData() const noexcept
    {
        return (sugoi::slice_data_t const*)(data() + structure->sliceDataOffsets[pt]);
    }

    SUGOI_FORCEINLINE sugoi::slice_data_t* getSliceData() noexcept
    {
        return (sugoi::slice_data_t*)(data() + structure->sliceDataOffsets[pt]);
    }

    SUGOI_FORCEINLINE sugoi::slice_lock_t& getSliceLock(const sugoi_type_index_t& type) const noexcept
    {
        const auto id = structure->index(type);
        const auto pData = getSliceData();
        return pData[id].lck;
    }
};

static_assert(sizeof(sugoi_chunk_t) <= sizeof(uint64_t) * 8, "Requires sugoi_chunk_t to match single cacheline!");