#pragma once
#include "render_graph/api.h"
#include "skr_renderer/primitive_draw.h"
#include "skr_renderer/skr_renderer.h"
#include "skr_renderer/mesh_resource.h"
#include "live2d_helpers.hpp"

#include "tracy/Tracy.hpp"

const skr_render_pass_name_t live2d_mask_pass_name = "Live2DMaskPass";

struct MaskPassLive2D : public IPrimitiveRenderPass {
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
        return live2d_mask_pass_name;
    }
};