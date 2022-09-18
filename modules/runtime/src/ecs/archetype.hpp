#pragma once

#include "ecs/dual.h"

namespace dual
{
// chunk data layout descriptor
struct archetype_t {
    struct dual_storage_t* storage;
    dual_type_set_t type;
    uint32_t* sizes;
    uint32_t* offsets[3];
    uint32_t* elemSizes;
    uint32_t* aligns;
    uint32_t versionOffset[3];
    dual_callback_v* callbacks;
    uint32_t chunkCapacity[3];
    uint32_t entitySize;
    uint32_t sizeToPatch;
    bool withMask;
    bool withDirty;

    /*
        uint32_t offsets[3][firstTag];
        uint32_t sizes[firstTag];
        dual_callback_v callbacks[firstTag - firstManaged];
    */

    SIndex index(dual_type_index_t type) const noexcept;
};
} // namespace dual

// group chunks by archetype and meta
struct dual_group_t {
    dual_chunk_t* firstChunk;
    dual_chunk_t* lastChunk;
    dual_chunk_t* firstFree;
    uint16_t chunkCount;
    uint32_t timestamp;
    uint32_t size;
    dual_entity_type_t type;
    dual::archetype_t* archetype;
    dual_group_t* dead;
    dual_group_t* cloned;

    bool isDead;
    bool disabled;
    /*
        type_index_t types[componentCount];
        entity metas[metaCount];
    */

    TIndex index(dual_type_index_t type) const noexcept;
    bool share(dual_type_index_t type) const noexcept;
    bool own(const dual_type_set_t& subtype) const noexcept;
    bool share(const dual_type_set_t& subtype) const noexcept;
    dual_mask_component_t get_shared_mask(const dual_type_set_t& subtype) const noexcept;
    void get_shared_type(dual_type_set_t& result, void* buffer) const noexcept;
    const dual_group_t* get_owner(dual_type_index_t type) const noexcept;
    dual_mask_component_t get_mask(const dual_type_set_t& subtype) const noexcept;
    const void* get_shared_ro(dual_type_index_t type) const noexcept;
    size_t data_size();

    void clear();

    dual_chunk_t* new_chunk(uint32_t hint);
    void add_chunk(dual_chunk_t* chunk);
    void remove_chunk(dual_chunk_t* chunk);

    void resize_chunk(dual_chunk_t* chunk, EIndex newSize);
    void mark_free(dual_chunk_t* chunk);
    void mark_full(dual_chunk_t* chunk);
};