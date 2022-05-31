#pragma once
#include <vector>
#include "dual.h"
namespace dual
{
struct entity_registry_t {
    struct entry_t {
        dual_chunk_t* chunk;
        uint32_t indexInChunk : 24;
        uint32_t version : 8;
    };
    std::vector<entry_t> entries;
    std::vector<EIndex> freeEntries;

    void reset();
    void shrink();
    void new_entities(dual_entity_t* dst, EIndex count);
    void free_entities(const dual_entity_t* dst, EIndex count);
    void fill_entities(const dual_chunk_view_t& view);
    void fill_entities(const dual_chunk_view_t& view, const dual_entity_t* src);
    void free_entities(const dual_chunk_view_t& view);
    void move_entities(const dual_chunk_view_t& view, const dual_chunk_t* src, EIndex srcIndex);
    void move_entities(const dual_chunk_view_t& view, EIndex srcIndex);
};
} // namespace dual