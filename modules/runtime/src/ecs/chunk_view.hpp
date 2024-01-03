#pragma once
#include "SkrRT/ecs/sugoi.h"

namespace sugoi
{

    void construct_view(const sugoi_chunk_view_t& view) noexcept;
    void destruct_view(const sugoi_chunk_view_t& view) noexcept;
    void construct_chunk(sugoi_chunk_t* chunk) noexcept;
    void destruct_chunk(sugoi_chunk_t* chunk) noexcept;
    void move_view(const sugoi_chunk_view_t& dst, EIndex srcIndex) noexcept;
    void move_view(const sugoi_chunk_view_t& dst, const sugoi_chunk_t* src, uint32_t srcIndex) noexcept;
    void cast_view(const sugoi_chunk_view_t& dst, sugoi_chunk_t* src, EIndex srcIndex) noexcept;
    void duplicate_view(const sugoi_chunk_view_t& dst, const sugoi_chunk_t* src, EIndex srcIndex) noexcept;
    void clone_view(const sugoi_chunk_view_t& dst, const sugoi_chunk_t* src, EIndex srcIndex) noexcept;
    template<class F>
    void iterator_ref_view(const sugoi_chunk_view_t& s, F&& iter) noexcept;
    template<class F>
    void iterator_ref_chunk(sugoi_chunk_t* chunk, F&& iter) noexcept;
    bool full_view(const sugoi_chunk_view_t& view) noexcept;
    const sugoi_entity_t* get_entities(const sugoi_chunk_view_t& view);
    void enable_components(const sugoi_chunk_view_t& view, const sugoi_type_set_t&  type);
    void disable_components(const sugoi_chunk_view_t& view, const sugoi_type_set_t&  type);
}