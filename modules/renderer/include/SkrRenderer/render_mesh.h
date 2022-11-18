#pragma once
#include "resources/mesh_resource.h"
#include "primitive_draw.h"
#include "cgpu/api.h"
#include "cgpu/io.h"
#ifndef __meta__
    #include "SkrRenderer/render_mesh.generated.h"
#endif

#ifdef __cplusplus
struct skr_render_mesh_t {
    skr_mesh_resource_id mesh_resource_id;

    eastl::vector<CGPUBufferId> buffers;
    eastl::vector<skr_vertex_buffer_view_t> vertex_buffer_views;
    eastl::vector<skr_index_buffer_view_t> index_buffer_views;
    eastl::vector<skr_render_primitive_command_t> primitive_commands;
};
#endif

typedef struct skr_render_primitive_command_t skr_render_primitive_command_t;
typedef struct skr_render_mesh_t skr_render_mesh_t;
typedef struct skr_render_mesh_t* skr_render_mesh_id;

struct sreflect sattr(
    "guid" : "c66ab7ef-bde9-4e0f-8023-a2d99ba5134c",
    "component" : true
) skr_render_mesh_comp_t {
    skr_resource_handle_t mesh_resource;
};
typedef struct skr_render_mesh_comp_t skr_render_mesh_comp_t;

SKR_RENDERER_EXTERN_C SKR_RENDERER_API 
void skr_render_mesh_free(skr_render_mesh_id render_mesh);

SKR_RENDERER_EXTERN_C SKR_RENDERER_API
void skr_render_mesh_initialize(skr_render_mesh_id render_mesh, skr_mesh_resource_id mesh_resource);