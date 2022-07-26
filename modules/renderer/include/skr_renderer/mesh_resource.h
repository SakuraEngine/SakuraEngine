#pragma once
#include "SkrRenderer/skr_renderer.configure.h"
#include "platform/guid.h"
#include "utils/types.h"

typedef uint64_t skr_vertex_layout_id;

struct sreflect sattr(
    "guid" : "3f01f94e-bd88-44a0-95e8-94ff74d18fca"
)
skr_vertex_buffer_entry_t
{
    uint32_t buffer_index;
    uint64_t stride;
    uint64_t offset;
};
typedef struct skr_vertex_buffer_entry_t skr_vertex_buffer_entry_t;

struct sreflect sattr(
    "guid" : "6ac5f946-dd65-4710-8725-ab4273fe13e6"
)
skr_index_buffer_entry_t
{
    uint32_t buffer_index;
    uint32_t index_offset;
    uint64_t first_index;
    uint64_t index_count;
    uint32_t stride;
};
typedef struct skr_index_buffer_entry_t skr_index_buffer_entry_t;

#ifdef __cplusplus
#include <EASTL/vector.h>

struct skr_mesh_primitive_t {
    skr_vertex_layout_id vertex_layout_id;
    skr_guid_t material_inst;
    eastl::vector<skr_vertex_buffer_entry_t> vertex_buffers;
    struct skr_index_buffer_entry_t index_buffer;
};

struct skr_mesh_section_t {
    int32_t parent_index;
    skr_float3_t translation;
    skr_float3_t scale;
    skr_float4_t rotation;
    eastl::vector<uint32_t> primive_indices;
};

struct sreflect sattr(
    "guid" : "03104e51-c998-410b-9d3c-d76535933440",
    "serialize" : "bin"
)
skr_mesh_bin_t
{
    skr_blob_t bin;
    bool used_with_index;
    bool used_with_vertex;
    eastl::string uri;
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
    eastl::vector<skr_mesh_bin_t> bins;
    void* gltf_data;
};
#endif
typedef struct skr_mesh_bin_t skr_mesh_bin_t;
typedef struct skr_mesh_primitive_t skr_mesh_primitive_t;
typedef struct skr_mesh_section_t skr_mesh_section_t;
typedef struct skr_mesh_resource_t skr_mesh_resource_t;
typedef struct skr_mesh_resource_t* skr_mesh_resource_id;

#include "utils/io.h"

typedef struct skr_gltf_ram_io_request_t {
    struct skr_vfs_t* vfs_override;
    bool load_bin_to_memory;
    skr_async_io_request_t ioRequest;
    SAtomic32 gltf_status;
    skr_mesh_resource_id mesh_resource;
#ifdef __cplusplus
    bool is_ready() const SKR_NOEXCEPT
    {
        return get_status() == SKR_ASYNC_IO_STATUS_OK;
    }
    SkrAsyncIOStatus get_status() const SKR_NOEXCEPT
    {
        return (SkrAsyncIOStatus)skr_atomic32_load_acquire(&gltf_status);
    }
#endif
} skr_gltf_ram_io_request_t;

#ifndef SKR_SERIALIZE_GURAD
SKR_RENDERER_EXTERN_C SKR_RENDERER_API void 
skr_mesh_resource_create_from_gltf(skr_io_ram_service_t* ioService, const char* path, skr_gltf_ram_io_request_t* request);
#endif

SKR_RENDERER_EXTERN_C SKR_RENDERER_API void 
skr_mesh_resource_free(skr_mesh_resource_id mesh_resource);

SKR_RENDERER_EXTERN_C SKR_RENDERER_API bool 
skr_mesh_resource_query_vertex_layout(skr_vertex_layout_id id, struct CGPUVertexLayout* out_vertex_layout);

SKR_RENDERER_EXTERN_C SKR_RENDERER_API skr_vertex_layout_id 
skr_mesh_resource_register_vertex_layout(const struct CGPUVertexLayout* in_vertex_layout);