#pragma once
#include "dual.h"

#ifdef __cplusplus
extern "C" {
#endif

RUNTIME_API dual_entity_t dualX_hashset_insert(
    dual_storage_t* storage, const dual_type_set_t* key_set,
    dual_entity_type_t* alloc_type, dual_view_callback_t callback, void* u);

#ifdef __cplusplus
}
#endif