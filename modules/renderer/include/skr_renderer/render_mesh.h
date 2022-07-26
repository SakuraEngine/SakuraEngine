#pragma once
#include "mesh_resource.h"
#include "primitive_draw.h"
#include "cgpu/api.h"
#include "cgpu/io.h"

#ifdef __cplusplus
#include "containers/span.hpp"

struct skr_render_primitive_command_t {
    skr::span<const skr_vertex_buffer_view_t> vbvs;
    const skr_vertex_buffer_view_t* ibv;
    uint64_t characteristic;
};

struct skr_render_mesh_t {
    skr_mesh_resource_id mesh_resource_id;
    eastl::vector<skr_async_io_request_t> vram_requests;
    CGPUBufferId gpu_buffer;
    eastl::vector<skr_vertex_buffer_view_t> vertex_buffer_views;
    eastl::vector<skr_index_buffer_view_t> index_buffer_views;
    eastl::vector<skr_render_primitive_command_t> primitive_commands;
};
#endif
typedef struct skr_render_primitive_command_t skr_render_primitive_command_t;
typedef struct skr_render_mesh_t skr_render_mesh_t;
typedef struct skr_render_mesh_t* skr_render_mesh_id;

typedef struct skr_render_mesh_request_t {
    skr_async_io_request_t ram_request;
    skr_render_mesh_id render_mesh;
    SAtomic32 status;
} skr_render_mesh_request_t;

struct sreflect sattr(
    "guid" : "c66ab7ef-bde9-4e0f-8023-a2d99ba5134c",
    "component" : true
) skr_render_mesh_comp_t {
    skr_guid_t resource_guid;
    skr_render_mesh_request_t async_request;
};
typedef struct skr_render_mesh_comp_t skr_render_mesh_comp_t;

#ifndef SKR_SERIALIZE_GURAD
SKR_RENDERER_EXTERN_C SKR_RENDERER_API void 
skr_render_mesh_create_from_gltf(skr_io_ram_service_t*, skr_io_vram_service_t*, const char* path, skr_render_mesh_request_t* request);
#endif

SKR_RENDERER_EXTERN_C SKR_RENDERER_API void 
skr_render_mesh_free(skr_render_mesh_id render_mesh);