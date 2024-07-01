#pragma once
#include "SkrRT/ecs/sugoi.h"
#include "SkrRenderer/skr_renderer.h"
#include "SkrRenderer/render_effect.h"
#include "SkrRT/ecs/type_builder.hpp"
#include "SkrContainers/vector.hpp"

namespace game {

typedef struct ForwardEffectToken {
    sugoi_entity_t game_entity;
} ForwardEffectToken;

static const skr_render_effect_name_t forward_effect_name = u8"ForwardEffect";

struct RenderEffectForward : public IRenderEffectProcessor 
{
    RenderEffectForward(skr_vfs_t* resource_vfs)
        :resource_vfs(resource_vfs) {}
    ~RenderEffectForward() = default;

    void on_register(SRendererId renderer, sugoi_storage_t* storage) override;
    void on_unregister(SRendererId renderer, sugoi_storage_t* storage) override;
    void get_type_set(const sugoi_chunk_view_t* cv, sugoi_type_set_t* set) override;
    sugoi_type_index_t get_identity_type() override;
    void initialize_data(SRendererId renderer, sugoi_storage_t* storage, sugoi_chunk_view_t* game_cv, sugoi_chunk_view_t* render_cv) override;
    skr_primitive_draw_packet_t produce_draw_packets(const skr_primitive_draw_context_t* context) override;

protected:
    void initialize_queries(sugoi_storage_t* storage);
    void release_queries();

    // TODO: move these anywhere else
    void prepare_geometry_resources(SRendererId renderer);
    void free_geometry_resources(SRendererId renderer);
    void prepare_pipeline(SRendererId renderer);
    void free_pipeline(SRendererId renderer);
    //

    sugoi::TypeSetBuilder type_builder;
    sugoi_type_set_t typeset;
    skr_vfs_t* resource_vfs;

    skr::Vector<skr_primitive_draw_t> mesh_drawcalls;
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
    sugoi_query_t* mesh_query = nullptr;
    sugoi_query_t* mesh_write_query = nullptr;
    sugoi_query_t* draw_mesh_query = nullptr;
    sugoi_query_t* draw_skin_query = nullptr;
    sugoi_type_index_t identity_type = {};
    struct PushConstants {
        skr_float4x4_t model;
    };
    skr::Vector<PushConstants> push_constants;
    skr::Vector<skr_float4x4_t> model_matrices;
};

}