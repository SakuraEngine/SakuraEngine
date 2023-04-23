#pragma once
#include "SkrRenderer/render_effect.h"

const ECGPUFormat depth_format = CGPU_FORMAT_D32_SFLOAT_S8_UINT;

static const skr_render_pass_name_t forward_pass_name = "ForwardPass";
struct RenderPassForward : public IPrimitiveRenderPass 
{
    void on_update(const skr_primitive_pass_context_t* context) override;
    void post_update(const skr_primitive_pass_context_t* context) override;
    void execute(const skr_primitive_pass_context_t* context, skr::span<const skr_primitive_draw_packet_t> drawcalls) override;    

    skr_render_pass_name_t identity() const override
    {
        return forward_pass_name;
    }

    dual::query_t anim_query;
    bool need_clear = true;
    
    ECGPUShadingRate shading_rate = CGPU_SHADING_RATE_FULL;
};