#pragma once

#include "ecs/constants.hpp"
#include "entity.hpp"

namespace dual
{
struct archetype_t;
}
struct dual_group_t;
struct dual_chunk_t {
    dual_chunk_t(dual::pool_type_t pt)
        : pt(pt)
    {
    }
    dual_chunk_t *next = nullptr, *prev = nullptr;
    dual::archetype_t* type = nullptr;
    dual_group_t* group = nullptr;
    EIndex count = 0;
    dual::pool_type_t pt;

    char* data() { return (char*)(this + 1); }
    char* data() const { return (char*)(this + 1); }
    uint32_t* timestamps() noexcept;
    const dual_entity_t* get_entities() const;
    EIndex get_capacity();

    static dual_chunk_t* create(dual::pool_type_t poolType);
    static void destroy(dual_chunk_t* chunk);
    void link(dual_chunk_t*) noexcept;
    void unlink() noexcept;
};