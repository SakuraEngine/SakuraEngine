#pragma once
#include "platform/guid.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct skr_type_t skr_type_t;
typedef skr_guid_t skr_type_id_t;

skr_type_t* skr_get_type(const skr_type_id_t* id);
void skr_get_derived_types(const skr_type_t* type, void (*callback)(void* u, skr_type_t* type), void* u);
void skr_get_type_id(const skr_type_t* type, skr_type_id_t* id);
uint32_t skr_get_type_size(const skr_type_t* type);

/*
generated:
skr_type_t* skr_typeof_xxxx();
void skr_typeid_xxxx(skr_type_id_t* id);
*/

#if defined(__cplusplus)
}
#endif