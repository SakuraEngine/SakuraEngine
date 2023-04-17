#pragma once
#include "resources/mesh_resource.h"
#include "SkrRenderer/render_effect.h"
#ifndef __meta__
    #include "SkrRenderer/render_mesh.generated.h"
#endif

#ifdef __cplusplus
namespace skr
{
namespace renderer
{
struct RenderMesh 
{
    skr_mesh_resource_id mesh_resource_id;
    skr::vector<CGPUBufferId> buffers;
    skr::vector<skr_vertex_buffer_view_t> vertex_buffer_views;
    skr::vector<skr_index_buffer_view_t> index_buffer_views;
    skr::vector<PrimitiveCommand> primitive_commands;
};
} // namespace renderer
} // namespace skr
#endif

sreflect_struct("guid" : "c66ab7ef-bde9-4e0f-8023-a2d99ba5134c")
sattr("component" : true)
skr_render_mesh_comp_t 
{
    SKR_RESOURCE_FIELD(skr_mesh_resource_t, mesh_resource);
};
typedef struct skr_render_mesh_comp_t skr_render_mesh_comp_t;

SKR_RENDERER_EXTERN_C SKR_RENDERER_API
void skr_render_mesh_initialize(skr_render_mesh_id render_mesh, skr_mesh_resource_id mesh_resource);

SKR_RENDERER_EXTERN_C SKR_RENDERER_API 
void skr_render_mesh_free(skr_render_mesh_id render_mesh);

// mesh render effect is a base class which captures components with render_mesh components and produce drawcalls
typedef struct SKR_RENDERER_API IMeshRenderEffect : public IRenderEffectProcessor {
#ifdef __cplusplus
    virtual ~IMeshRenderEffect() = default;

    virtual skr_primitive_draw_packet_t produce_draw_packets(const skr_primitive_draw_context_t* context) SKR_NOEXCEPT;
#endif
} IMeshRenderEffect;