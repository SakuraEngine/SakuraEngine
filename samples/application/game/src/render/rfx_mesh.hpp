#pragma once
#include "ecs/dual.h"

#include "SkrRenderer/skr_renderer.h"
#include "SkrRenderer/render_effect.h"
#include "ecs/type_builder.hpp"

typedef struct forward_effect_identity_t {
    dual_entity_t game_entity;
} forward_effect_identity_t;

static const skr_render_effect_name_t forward_effect_name = u8"ForwardEffect";

struct RenderEffectForward : public IRenderEffectProcessor 
{
    RenderEffectForward(skr_vfs_t* resource_vfs)
        :resource_vfs(resource_vfs) {}
    ~RenderEffectForward() = default;

    void on_register(SRendererId renderer, dual_storage_t* storage) override;
    void on_unregister(SRendererId renderer, dual_storage_t* storage) override;
    void get_type_set(const dual_chunk_view_t* cv, dual_type_set_t* set) override;
    dual_type_index_t get_identity_type() override;
    void initialize_data(SRendererId renderer, dual_storage_t* storage, dual_chunk_view_t* game_cv, dual_chunk_view_t* render_cv) override;
    skr_primitive_draw_packet_t produce_draw_packets(const skr_primitive_draw_context_t* context) override;

protected:
    void initialize_queries(dual_storage_t* storage);
    void release_queries();

    // TODO: move these anywhere else
    void prepare_geometry_resources(SRendererId renderer);
    void free_geometry_resources(SRendererId renderer);
    void prepare_pipeline(SRendererId renderer);
    void free_pipeline(SRendererId renderer);
    //

    dual::type_builder_t type_builder;
    dual_type_set_t typeset;
    skr_vfs_t* resource_vfs;

    eastl::vector<skr_primitive_draw_t> mesh_drawcalls;
    skr_primitive_draw_list_view_t mesh_draw_list;
    
protected:
    // render resources
    skr_vertex_buffer_view_t vbvs[5];
    skr_index_buffer_view_t ibv;
    CGPUBufferId vertex_buffer;
    CGPUBufferId index_buffer;
    CGPURenderPipelineId pipeline;
    CGPURenderPipelineId skin_pipeline;
    // effect processor data
    const char8_t* push_constants_name = u8"push_constants";
    dual_query_t* mesh_query = nullptr;
    dual_query_t* mesh_write_query = nullptr;
    dual_query_t* draw_mesh_query = nullptr;
    dual_query_t* draw_skin_query = nullptr;
    dual_type_index_t identity_type = {};
    struct PushConstants {
        skr_float4x4_t model;
    };
    eastl::vector<PushConstants> push_constants;
    eastl::vector<skr_float4x4_t> model_matrices;
};