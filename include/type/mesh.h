#pragma once
#include "platform/configure.h"
#include "utils/types.h"

typedef const char* skr_vertex_prop_name_t;
typedef const char* skr_mesh_name_t;

typedef struct skr_indices_t {
    skr_blob_t data;
} skr_indices_t;

typedef struct skr_vertex_props_t {
    skr_vertex_prop_name_t name;
    skr_blob_t data;
} skr_vertex_props_t;

typedef skr_vertex_props_t* skr_vertex_props_id;
#define SKR_VERTEX_PROP_ID_INVALID 0
#define SKR_MESH_MAX_VERTEX_PROPS 15

typedef struct skr_mesh_t {
    skr_vertex_props_id vertex_prop_ids[SKR_MESH_MAX_VERTEX_PROPS];
    uint32_t vertex_prop_count;
    skr_indices_t indices;
} skr_mesh_t;
typedef skr_mesh_t* skr_mesh_id_t;

// RUNTIME_EXTERN_C RUNTIME_API 