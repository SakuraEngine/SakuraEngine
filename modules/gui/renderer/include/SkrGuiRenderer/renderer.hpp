#pragma once
#include "SkrGuiRenderer/module.configure.h"
#include "SkrGui/gdi.h"
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include <containers/vector.hpp>
#include "rtm/rtmx.h"

namespace skr {
namespace gdi {

struct SGDIElementDrawCommand_RenderGraph
{
    // texture
    // material
    uint32_t first_index;
    uint32_t index_count;
    uint64_t ib_offset;
    uint64_t vb_offset;
    uint64_t tb_offset;
    uint64_t pb_offset;
};

struct SKR_GUI_RENDERER_API SGDICanvasGroupData_RenderGraph
{
    inline SGDICanvasGroupData_RenderGraph(SGDICanvasGroup* group)
        : canvas_group(group)
    {

    }

    SGDICanvasGroup* canvas_group = nullptr;

    skr::vector<skr::render_graph::BufferHandle> vertex_buffers;
    skr::vector<skr::render_graph::BufferHandle> transform_buffers;
    skr::vector<skr::render_graph::BufferHandle> projection_buffers;
    skr::vector<skr::render_graph::BufferHandle> index_buffers;

    skr::vector<SGDIElementDrawCommand_RenderGraph> render_commands;
    skr::vector<SGDIVertex> render_vertices;
    skr::vector<rtm::matrix4x4f> render_transforms;
    skr::vector<rtm::matrix4x4f> render_projections;
    skr::vector<index_t> render_indices;
};

struct SGDIRendererDescriptor_RenderGraph
{
    CGPUDeviceId device = nullptr;
};

struct SGDIRenderParams_RenderGraph
{
    skr::render_graph::RenderGraph* render_graph = nullptr;
};

struct SKR_GUI_RENDERER_API SGDIRenderer_RenderGraph : public SGDIRenderer
{
    int initialize(const SGDIRendererDescriptor* desc) SKR_NOEXCEPT final;
    int finalize() SKR_NOEXCEPT final;
    void render(SGDICanvasGroup* canvas_group, SGDIRenderParams* params) SKR_NOEXCEPT final;

    CGPUVertexLayout vertex_layout = {};
    CGPURenderPipelineId pipeline = nullptr;
};

} }