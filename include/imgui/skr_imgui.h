#pragma once
#include "platform/window.h"
#include "platform/input.h"
#include "render_graph/frontend/render_graph.hpp"

typedef struct RenderGraphImGuiDescriptor {
    sakura::render_graph::RenderGraph* render_graph;
    CGPUQueueId queue;
    CGPUSamplerId static_sampler;
    CGPUPipelineShaderDescriptor vs;
    CGPUPipelineShaderDescriptor ps;
    ECGPUFormat backbuffer_format;
} RenderGraphImGuiDescriptor;

RUNTIME_API void render_graph_imgui_initialize(const RenderGraphImGuiDescriptor* desc);
RUNTIME_API void render_graph_imgui_add_render_pass(
sakura::render_graph::RenderGraph* render_graph,
sakura::render_graph::TextureRTVHandle target,
ECGPULoadAction load_action);
RUNTIME_API void render_graph_imgui_finalize();
RUNTIME_API void skr_imgui_new_frame(SWindowHandle window, float delta_time);
