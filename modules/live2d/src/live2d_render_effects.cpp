#include "platform/memory.h"
#include "utils/make_zeroed.hpp"
#include "ecs/type_builder.hpp"
#include "skr_live2d/skr_live2d.h"
#include "skr_live2d/render_effect.h"

#include "live2d_helpers.hpp"
#include "Framework/Math/CubismMatrix44.hpp"
#include "Framework/Math/CubismViewMatrix.hpp"

#include "skr_renderer/primitive_draw.h"
#include "skr_renderer/skr_renderer.h"
#include "skr_renderer/mesh_resource.h"

skr_render_pass_name_t live2d_pass_name = "Live2DPass";
struct RenderPassLive2D : public IPrimitiveRenderPass {
    void on_register(ISkrRenderer* renderer) override
    {

    }

    void on_unregister(ISkrRenderer* renderer) override
    {

    }

    void execute(skr::render_graph::RenderGraph* renderGraph, skr_primitive_draw_list_view_t drawcalls) override
    {

    }

    skr_render_pass_name_t identity() const override
    {
        return live2d_pass_name;
    }
};
RenderPassLive2D* live2d_pass = new RenderPassLive2D();

skr_render_effect_name_t live2d_effect_name = "Live2DEffect";
typedef struct live2d_effect_identity_t {
    dual_entity_t game_entity;
} live2d_effect_identity_t;
struct RenderEffectLive2D : public IRenderEffectProcessor {
    ~RenderEffectLive2D() = default;

    dual_query_t* effect_query = nullptr;
    dual::type_builder_t type_builder;
    dual_type_index_t identity_type = {};

    void on_register(ISkrRenderer* renderer, dual_storage_t* storage) override
    {
        // make identity component type
        {
            auto guid = make_zeroed<skr_guid_t>();
            dual_make_guid(&guid);
            auto desc = make_zeroed<dual_type_description_t>();
            desc.name = "live2d_identity";
            desc.size = sizeof(live2d_effect_identity_t);
            desc.guid = guid;
            desc.alignment = alignof(live2d_effect_identity_t);
            identity_type = dualT_register_type(&desc);
        }
        type_builder.with(identity_type);
        // type_builder.with<skr_render_mesh_comp_t>();
        effect_query = dualQ_from_literal(storage, "[in]live2d_identity");
        // prepare render resources
        prepare_pipeline(renderer);
        // prepare_geometry_resources(renderer);
    }

    void on_unregister(ISkrRenderer* renderer, dual_storage_t* storage) override
    {
        free_pipeline(renderer);
    }

    void get_type_set(const dual_chunk_view_t* cv, dual_type_set_t* set) override
    {
        *set = type_builder.build();
    }

    dual_type_index_t get_identity_type() override
    {
        return identity_type;
    }

    void initialize_data(ISkrRenderer* renderer, dual_storage_t* storage, dual_chunk_view_t* game_cv, dual_chunk_view_t* render_cv) override
    {

    }

    uint32_t produce_drawcall(IPrimitiveRenderPass* pass, dual_storage_t* storage) override
    {
        return 0;
    }

    void peek_drawcall(IPrimitiveRenderPass* pass, skr_primitive_draw_list_view_t* drawcalls, dual_storage_t* storage) override
    {
        
    }
protected:
    void prepare_pipeline(ISkrRenderer* renderer);
    void free_pipeline(ISkrRenderer* renderer);
};
RenderEffectLive2D* live2d_effect = new RenderEffectLive2D();

void RenderEffectLive2D::prepare_pipeline(ISkrRenderer* renderer)
{

}

void RenderEffectLive2D::free_pipeline(ISkrRenderer* renderer)
{

}

void skr_live2d_initialize_render_effects(live2d_render_graph_t* render_graph)
{
    auto renderer = skr_renderer_get_renderer();
    skr_renderer_register_render_pass(renderer, live2d_pass_name, live2d_pass);
    skr_renderer_register_render_effect(renderer, live2d_effect_name, live2d_effect);
}

void skr_live2d_finalize_render_effects(live2d_render_graph_t* render_graph)
{
    auto renderer = skr_renderer_get_renderer();
    skr_renderer_remove_render_pass(renderer, live2d_pass_name);
    skr_renderer_remove_render_effect(renderer, live2d_effect_name);
    delete live2d_effect;
    delete live2d_pass;
}