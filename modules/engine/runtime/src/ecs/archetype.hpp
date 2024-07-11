#pragma once
#include "SkrRT/ecs/sugoi.h"
#include "SkrContainers/stl_vector.hpp"

namespace sugoi
{
struct resource_fields_t
{
    intptr_t offsets;
    uint32_t count;
};

// chunk data layout descriptor
struct SKR_RUNTIME_API archetype_t {
    sugoi_storage_t* const storage;
    const sugoi_type_set_t type;
    const uint32_t* const sizes;
    const uint32_t* const offsets[3];
    const uint32_t* const elemSizes;
    const uint32_t* const aligns;

    const uint32_t sliceDataOffsets[3];
    const uint32_t* const callbackFlags;
    const uint32_t* const stableOrder;
    const sugoi_callback_v* const callbacks;
    const resource_fields_t* const resourceFields;
    const uint32_t chunkCapacity[3];
    const uint32_t entitySize;
    const uint32_t sizeToPatch = 0;
    const uint32_t firstChunkComponent; //chunk component count
    const bool withMask = false;
    const bool withDirty = false;
    /*
        uint32_t offsets[3][firstTag];
        uint32_t sizes[firstTag];
        sugoi_callback_v callbacks[firstTag - firstManaged];
    */
    bool with_chunk_component() const noexcept;
    SIndex index(sugoi_type_index_t type) const noexcept;
};
} // namespace sugoi

// group chunks by archetype and meta
struct sugoi_group_t {
    skr::stl_vector<sugoi_chunk_t*> chunks;
    uint32_t firstFree;
    uint32_t timestamp;
    uint32_t size;
    sugoi_entity_type_t type;
    sugoi::archetype_t* archetype;
    sugoi_group_t* dead;
    sugoi_group_t* cloned;

    bool isDead;
    bool disabled;
    /*
        type_index_t types[componentCount];
        entity metas[metaCount];
    */

    TIndex index(sugoi_type_index_t type) const noexcept;
    bool share(sugoi_type_index_t type) const noexcept;
    bool own(const sugoi_type_set_t& subtype) const noexcept;
    bool share(const sugoi_type_set_t& subtype) const noexcept;
    sugoi_mask_comp_t get_shared_mask(const sugoi_type_set_t& subtype) const noexcept;
    void get_shared_type(sugoi_type_set_t& result, void* buffer) const noexcept;
    const sugoi_group_t* get_owner(sugoi_type_index_t type) const noexcept;
    sugoi_mask_comp_t get_mask(const sugoi_type_set_t& subtype) const noexcept;
    const void* get_shared_ro(sugoi_type_index_t type) const noexcept;
    size_t data_size();

    void clear();

    sugoi_chunk_t* get_first_free_chunk() const noexcept;
    sugoi_chunk_t* new_chunk(uint32_t hint);
    void add_chunk(sugoi_chunk_t* chunk);
    void remove_chunk(sugoi_chunk_t* chunk);

    void resize_chunk(sugoi_chunk_t* chunk, EIndex newSize);
    void mark_free(sugoi_chunk_t* chunk);
    void mark_full(sugoi_chunk_t* chunk);
};