#pragma once
#include "ecs/dual.h"
#include "type.hpp"

namespace dual
{

    void construct_view(const dual_chunk_view_t& view) noexcept;
    void destruct_view(const dual_chunk_view_t& view) noexcept;
    void construct_chunk(dual_chunk_t* chunk) noexcept;
    void destruct_chunk(dual_chunk_t* chunk) noexcept;
    void move_view(const dual_chunk_view_t& dst, EIndex srcIndex) noexcept;
    void move_view(const dual_chunk_view_t& dst, const dual_chunk_t* src, uint32_t srcIndex) noexcept;
    void cast_view(const dual_chunk_view_t& dst, dual_chunk_t* src, EIndex srcIndex) noexcept;
    void duplicate_view(const dual_chunk_view_t& dst, const dual_chunk_t* src, EIndex srcIndex) noexcept;
    template<class F>
    void iterator_ref_view(const dual_chunk_view_t& s, F&& iter) noexcept;
    template<class F>
    void iterator_ref_chunk(dual_chunk_t* chunk, F&& iter) noexcept;
    bool full_view(const dual_chunk_view_t& view) noexcept;
    const dual_entity_t* get_entities(const dual_chunk_view_t& view, type_index_t type);
    void enable_components(const dual_chunk_view_t& view, const dual_type_set_t&  type);
    void disable_components(const dual_chunk_view_t& view, const dual_type_set_t&  type);
}