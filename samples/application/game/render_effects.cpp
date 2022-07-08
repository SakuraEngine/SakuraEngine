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
#include "skr_renderer/render_effects/effect_processor.h"
#include "skr_renderer/skr_renderer.h"
#include <iostream>

struct RenderEffectForward : public IRenderEffectProcessor {
    ~RenderEffectForward() = default;

    void get_type_set(const dual_chunk_view_t* cv, dual_type_set_t* set) override
    {
    }

    void initialize_data(ISkrRenderer* renderer, dual_storage_t* storage, dual_chunk_view_t* cv) override
    {
    }

    uint32_t produce_drawcall(SGameSceneStorage* game_storage) override
    {
        return 0;
    }

    void peek_drawcall(skr_primitive_draw_list_view_t* drawcalls) override
    {
    }
};
RenderEffectForward* forward_effect = new RenderEffectForward();
skr_renderer_effect_name_t forward_effect_name = "Forward";

void initialize_render_effects(skr::render_graph::RenderGraph* renderGraph)
{
    auto renderer = skr_renderer_get_renderer();
    skr_renderer_register_render_effect(renderer, forward_effect_name, forward_effect);
}

void finalize_render_effects(skr::render_graph::RenderGraph* renderGraph)
{
    auto renderer = skr_renderer_get_renderer();
    skr_renderer_remove_render_effect(renderer, forward_effect_name);
}