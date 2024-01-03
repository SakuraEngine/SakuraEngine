#pragma once
#include "SkrRT/ecs/sugoi_types.h" // IWYU pragma: keep

namespace sugoi
{
struct archetype_t;
}
struct sugoi_group_t;
struct sugoi_chunk_t {
    sugoi_chunk_t(sugoi::pool_type_t pt)
        : pt(pt)
    {
    }
    uint32_t index;
    sugoi::archetype_t* type = nullptr;
    sugoi_group_t* group = nullptr;
    EIndex count = 0;
    sugoi::pool_type_t pt;

    char* data() { return (char*)(this + 1); }
    char* data() const { return (char*)(this + 1); }
    uint32_t* timestamps() noexcept;
    const sugoi_entity_t* get_entities() const;
    EIndex get_capacity();

    static sugoi_chunk_t* create(sugoi::pool_type_t poolType);
    static void destroy(sugoi_chunk_t* chunk);
};