#pragma once
#include "SkrRenderer/skr_renderer.configure.h"
#include "platform/guid.h"
#include "utils/types.h"
#ifdef __cplusplus
#include <EASTL/vector.h>
#endif

typedef uint64_t skr_vertex_layout_id;

struct skr_mesh_primitive_t {
    uint32_t index_offset;
    uint32_t first_index;
    uint32_t index_count;
    skr_vertex_layout_id vertex_layout_id;
    skr_guid_t material_inst;
};
typedef struct skr_mesh_primitive_t skr_mesh_primitive_t;

struct sreflect sattr(
    "guid" : "3f01f94e-bd88-44a0-95e8-94ff74d18fca",
    "serialize" : "bin"
)
skr_vertex_bin_t
{
    skr_blob_t blob;
};
typedef struct skr_vertex_bin_t skr_vertex_bin_t;

struct sreflect sattr(
    "guid" : "6ac5f946-dd65-4710-8725-ab4273fe13e6",
    "serialize" : "bin"
)
skr_index_bin_t
{
    skr_blob_t blob;
    uint8_t stride;
};
typedef struct skr_index_bin_t skr_index_bin_t;

#ifdef __cplusplus
struct skr_mesh_section_t {
    int32_t parent_index;
    skr_float3_t translation;
    skr_float3_t scale;
    skr_float4_t rotation;
    eastl::vector<uint32_t> primive_indices;
};

struct sreflect sattr(
    "guid" : "3b8ca511-33d1-4db4-b805-00eea6a8d5e1",
    "serialize" : "bin"
)
skr_mesh_resource_t
{
    eastl::string name;
    eastl::vector<skr_mesh_section_t> sections;
    eastl::vector<skr_mesh_primitive_t> primitives;
    eastl::vector<skr_vertex_bin_t> vertex_buffers;
    struct skr_index_bin_t index_buffer;
    uint32_t index_stride;
    void* gltf_data;
};
#endif
typedef struct skr_mesh_section_t skr_mesh_section_t;
typedef struct skr_mesh_resource_t skr_mesh_resource_t;
typedef struct skr_mesh_resource_t* skr_mesh_resource_id;

#include "utils/io.h"

typedef struct skr_gltf_ram_io_request_t {
    skr_vfs_t* vfs_override;
    skr_async_io_request_t ioRequest;
    SAtomic32 gltf_status;
    skr_mesh_resource_id mesh_resource;
} skr_gltf_ram_io_request_t;

SKR_RENDERER_EXTERN_C SKR_RENDERER_API void 
skr_mesh_resource_create_from_gltf(skr_io_ram_service_t* ioService, const char* name, skr_gltf_ram_io_request_t* request);

SKR_RENDERER_EXTERN_C SKR_RENDERER_API void 
skr_mesh_resource_free(skr_mesh_resource_id mesh_resource);

SKR_RENDERER_EXTERN_C SKR_RENDERER_API bool 
skr_mesh_resource_query_vertex_layout(skr_vertex_layout_id id, struct CGPUVertexLayout* out_vertex_layout);

SKR_RENDERER_EXTERN_C SKR_RENDERER_API skr_vertex_layout_id 
skr_mesh_resource_register_vertex_layout(const struct CGPUVertexLayout* in_vertex_layout);