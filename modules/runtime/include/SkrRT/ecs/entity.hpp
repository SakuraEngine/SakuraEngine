#pragma once
#include "SkrRT/ecs/sugoi_config.h"
#include <limits>

namespace sugoi
{
    
constexpr static sugoi_entity_t kEntityNull = std::numeric_limits<sugoi_entity_t>::max();
constexpr static sugoi_entity_t kEntityTransientVersion = ((1 << (sizeof(sugoi_entity_t) * 8 - SUGOI_ENTITY_VERSION_OFFSET)) - 1);

SUGOI_FORCEINLINE sugoi_entity_t e_id(sugoi_entity_t e)
{
    return e & SUGOI_ENTITY_ID_MASK;
}

SUGOI_FORCEINLINE sugoi_entity_t e_version(sugoi_entity_t e)
{
    return (e >> SUGOI_ENTITY_VERSION_OFFSET) & SUGOI_ENTITY_VERSION_MASK;
}

SUGOI_FORCEINLINE sugoi_entity_t e_id(sugoi_entity_t e, sugoi_entity_t value)
{
    return e_version(e) | e_id(value);
}

SUGOI_FORCEINLINE sugoi_entity_t e_version(sugoi_entity_t e, sugoi_entity_t value)
{
    return ((value & SUGOI_ENTITY_VERSION_MASK) << SUGOI_ENTITY_VERSION_OFFSET) | e_id(e);
}

SUGOI_FORCEINLINE bool e_transient(sugoi_entity_t e)
{
    return e_version(e) == kEntityTransientVersion;
}

SUGOI_FORCEINLINE sugoi_entity_t e_make_transient(sugoi_entity_t e)
{
    return e_version(e, kEntityTransientVersion);
}

SUGOI_FORCEINLINE sugoi_entity_t e_recycle(sugoi_entity_t e)
{
    auto v = e_version(e);
    return e_version(e, ((v + 1 == kEntityTransientVersion) ? 0 : (v + 1)));
}

SUGOI_FORCEINLINE sugoi_entity_t e_inc_version(sugoi_entity_t v)
{
    return ((v + 1 == kEntityTransientVersion) ? 0 : (v + 1));
}

} // namespace sugoi