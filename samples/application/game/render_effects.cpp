#include "../../cgpu/common/utils.h"
#include "platform/window.h"
#include "render_graph/frontend/render_graph.hpp"
#include "imgui/skr_imgui.h"
#include "imgui/imgui.h"
#include "gamert.h"
#include "platform/memory.h"
#include "utils/log.h"
#include "skr_renderer/skr_renderer.h"
#include "runtime_module.h"
#include "imgui/skr_imgui_rg.h"
#include "skr_renderer/effect_processor.h"
#include "skr_renderer/skr_renderer.h"
#include "math/vectormath.hpp"

skr_render_pass_name_t forward_pass_name = "ForwardPass";
struct RenderPassForward : public IPrimitiveRenderPass {
    void execute(skr::render_graph::RenderGraph* renderGraph, const skr_primitive_draw_list_view_t* dc) override
    {
    }
    skr_render_pass_name_t identity() const override
    {
        return forward_pass_name;
    }
};
RenderPassForward* forward_pass = new RenderPassForward();

skr_render_effect_name_t forward_effect_name = "ForwardEffect";
struct RenderEffectForward : public IRenderEffectProcessor {
    ~RenderEffectForward() = default;

    void get_type_set(const dual_chunk_view_t* cv, dual_type_set_t* set) override
    {
    }

    void initialize_data(ISkrRenderer* renderer, dual_storage_t* storage, dual_chunk_view_t* cv) override
    {
    }

    uint32_t produce_drawcall(IPrimitiveRenderPass* pass, dual_storage_t* game_storage) override
    {
        // SKR_LOG_FMT_INFO("Pass {} asked Feature {} to produce drawcall", pass->identity(), forward_effect_name);
        if (strcmp(pass->identity(), forward_pass_name) == 0)
        {
            return 0;
        }
        return 0;
    }

    void peek_drawcall(IPrimitiveRenderPass* pass, skr_primitive_draw_list_view_t* drawcalls) override
    {
        // SKR_LOG_FMT_INFO("Pass {} asked Feature {} to peek drawcall", pass->identity(), forward_effect_name);
        if (strcmp(pass->identity(), forward_pass_name) == 0)
        {
        }
    }
    struct PushConstants {
        skr::math::float4x4 world;
        skr::math::float4x4 view_proj;
    };
    eastl::vector<PushConstants> push_constants;
};
RenderEffectForward* forward_effect = new RenderEffectForward();

void initialize_render_effects(skr::render_graph::RenderGraph* renderGraph)
{
    auto renderer = skr_renderer_get_renderer();
    skr_renderer_register_render_pass(renderer, forward_pass_name, forward_pass);
    skr_renderer_register_render_effect(renderer, forward_effect_name, forward_effect);
}

void finalize_render_effects(skr::render_graph::RenderGraph* renderGraph)
{
    auto renderer = skr_renderer_get_renderer();
    skr_renderer_remove_render_pass(renderer, forward_pass_name);
    skr_renderer_remove_render_effect(renderer, forward_effect_name);
    delete forward_effect;
    delete forward_pass;
}