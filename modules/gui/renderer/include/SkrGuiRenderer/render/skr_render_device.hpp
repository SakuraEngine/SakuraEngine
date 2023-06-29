#pragma once
#include "SkrGuiRenderer/module.configure.h"
#include "cgpu/api.h"
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "platform/window.h"
#include "SkrGui/fwd_config.hpp"
#include <EASTL/vector_map.h>

namespace skr::gui
{
using skr::render_graph::RenderGraph;
struct SkrRenderWindow;
struct IUpdatableImage;

// 渲染设备，渲染的
struct SKR_GUI_RENDERER_API SkrRenderDevice final {
    enum EPipelineFlag
    {
        EPipelineFlag_None = 0,
        EPipelineFlag_TestZ = 1 << 0,
        EPipelineFlag_Textured = 1 << 1,
        EPipelineFlag_WriteZ = 1 << 2,
        EPipelineFlag_CustomSampler = 1 << 3,
        __Count = 4,
    };

    // init & shutdown
    void init();
    void shutdown();

    // create view
    SkrRenderWindow* create_window(SWindowHandle window);
    void             destroy_window(SkrRenderWindow* view);

    // getter
    inline CGPUInstanceId cgpu_instance() const { return _cgpu_instance; }
    inline CGPUAdapterId  cgpu_adapter() const { return _cgpu_adapter; }
    inline CGPUDeviceId   cgpu_device() const { return _cgpu_device; }
    inline CGPUQueueId    cgpu_queue() const { return _cgpu_queue; }
    inline RenderGraph*   render_graph() const { return _render_graph; }

    // pipeline
    CGPURenderPipelineId get_pipeline(EPipelineFlag flags, ECGPUSampleCount sample_count);
    CGPURenderPipelineId create_pipeline(EPipelineFlag flags, ECGPUSampleCount sample_count);

    // texture
    // create texture
    // update texture

private:
    // cgpu device
    CGPUInstanceId _cgpu_instance = nullptr;
    CGPUAdapterId  _cgpu_adapter = nullptr;
    CGPUDeviceId   _cgpu_device = nullptr;
    CGPUQueueId    _cgpu_queue = nullptr;

    // rg
    RenderGraph* _render_graph = nullptr;

    // PSO
    struct PipelineKey {
        EPipelineFlag    flags;
        ECGPUSampleCount sample_count;
        inline bool      operator==(const PipelineKey& other) const
        {
            return flags == other.flags && sample_count == other.sample_count;
        }
        inline bool operator!=(const PipelineKey& other) const
        {
            return flags != other.flags || sample_count != other.sample_count;
        }
        inline bool operator<(const PipelineKey& other) const
        {
            return flags < other.flags || (flags == other.flags && sample_count < other.sample_count);
        }
    };
    eastl::vector_map<PipelineKey, CGPURenderPipelineId> _pipelines;
    CGPURootSignaturePoolId                              _rs_pool;
    CGPUSamplerId                                        _static_color_sampler;
    CGPUVertexLayout                                     _vertex_layout = {};

    // Texture updates
    Array<IUpdatableImage*> _texture_updates;
};
} // namespace skr::gui