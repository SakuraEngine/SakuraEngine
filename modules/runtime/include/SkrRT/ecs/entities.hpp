#pragma once
#include "sugoi.h"
#include "SkrRT/platform/thread.h"
#include "SkrRT/containers/vector.hpp"

namespace sugoi
{
struct entity_registry_t {
    struct entry_t {
        sugoi_chunk_t* chunk;
        uint32_t indexInChunk : 24;
        uint32_t version : 8;
    };
    skr::Vector<entry_t> entries;
    skr::Vector<EIndex> freeEntries;
    SMutexObject mutex;

    void reset();
    void shrink();
    void new_entities(sugoi_entity_t* dst, EIndex count);
    void free_entities(const sugoi_entity_t* dst, EIndex count);
    void fill_entities(const sugoi_chunk_view_t& view);
    void fill_entities(const sugoi_chunk_view_t& view, const sugoi_entity_t* src);
    void free_entities(const sugoi_chunk_view_t& view);
    void move_entities(const sugoi_chunk_view_t& view, const sugoi_chunk_t* src, EIndex srcIndex);
    void move_entities(const sugoi_chunk_view_t& view, EIndex srcIndex);
};
} // namespace sugoi