#pragma once
#include "render_graph/frontend/render_graph.hpp"

typedef struct RenderGraphImGuiDescriptor {
    sakura::render_graph::RenderGraph* render_graph;
    CGpuQueueId queue;
    CGpuSamplerId static_sampler;
    CGpuPipelineShaderDescriptor vs;
    CGpuPipelineShaderDescriptor ps;
    ECGpuFormat backbuffer_format;
} RenderGraphImGuiDescriptor;

RUNTIME_API void render_graph_imgui_initialize(const RenderGraphImGuiDescriptor* desc);
RUNTIME_API void render_graph_imgui_setup_resources(sakura::render_graph::RenderGraph* render_graph);
RUNTIME_API void render_graph_imgui_add_render_pass(
    sakura::render_graph::RenderGraph* render_graph,
    sakura::render_graph::TextureRTVHandle target,
    ECGpuLoadAction load_action);
RUNTIME_API void render_graph_imgui_finalize();
