#pragma once
#include "ecs/dual_config.h"
#include <limits>
namespace dual
{
constexpr static dual_entity_t kEntityNull = std::numeric_limits<dual_entity_t>::max();
constexpr static dual_entity_t kEntityTransientVersion = ((1 << (sizeof(dual_entity_t) * 8 - DUAL_ENTITY_VERSION_OFFSET)) - 1);

DUAL_FORCEINLINE dual_entity_t e_id(dual_entity_t e)
{
    return e & DUAL_ENTITY_ID_MASK;
}
DUAL_FORCEINLINE dual_entity_t e_version(dual_entity_t e)
{
    return (e >> DUAL_ENTITY_VERSION_OFFSET) & DUAL_ENTITY_VERSION_MASK;
}
DUAL_FORCEINLINE dual_entity_t e_id(dual_entity_t e, dual_entity_t value)
{
    return e_version(e) | e_id(value);
}
DUAL_FORCEINLINE dual_entity_t e_version(dual_entity_t e, dual_entity_t value)
{
    return ((value & DUAL_ENTITY_VERSION_MASK) << DUAL_ENTITY_VERSION_OFFSET) | e_id(e);
}
DUAL_FORCEINLINE bool e_transient(dual_entity_t e)
{
    return e_version(e) == kEntityTransientVersion;
}
DUAL_FORCEINLINE dual_entity_t e_make_transient(dual_entity_t e)
{
    return e_version(e, kEntityTransientVersion);
}
DUAL_FORCEINLINE dual_entity_t e_recycle(dual_entity_t e)
{
    auto v = e_version(e);
    return e_version(e, ((v + 1 == kEntityTransientVersion) ? 0 : (v + 1)));
}
DUAL_FORCEINLINE dual_entity_t e_inc_version(dual_entity_t v)
{
    return ((v + 1 == kEntityTransientVersion) ? 0 : (v + 1));
}
} // namespace dual