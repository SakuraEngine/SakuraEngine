#pragma once
#include "platform/configure.h"
#include <EASTL/vector.h>
#include "resource/resource_handle.h"
#include "utils/types.h"

typedef struct skr_mesh_section_t {
    // attachment?
    // skr_mesh_section_t* parent;
    // eastl::vector<skr_mesh_section_t> children;
    uint32_t index;
    skr_float3_t translation;
    skr_float3_t scale;
    skr_float4_t rotation;
} skr_mesh_section_t;

typedef struct skr_mesh_primitive_t {
    uint32_t index_offset;
    uint32_t first_index;
    uint32_t index_count;
    uint32_t vertex_layout_id;
    skr_guid_t material_inst;
} skr_mesh_primitive_t;

struct reflect attr(
    "guid" : "3f01f94e-bd88-44a0-95e8-94ff74d18fca",
    "serialize" : "bin"
)
skr_vertex_bin_t {
    skr_bin_t blob;
};

struct reflect attr(
    "guid" : "6ac5f946-dd65-4710-8725-ab4273fe13e6",
    "serialize" : "bin"
)
skr_index_bin_t {
    skr_bin_t blob;
};

struct reflect attr(
    "guid" : "3b8ca511-33d1-4db4-b805-00eea6a8d5e1",
    "serialize" : "bin"
)
skr_mesh_resource_t
{
    eastl::vector<skr_mesh_section_t> sections;
    eastl::vector<skr_mesh_primitive_t> primitives;
    eastl::vector<skr_vertex_bin_t> vertex_buffers;
    struct skr_index_bin_t index_buffer;
    uint32_t index_stride;
};