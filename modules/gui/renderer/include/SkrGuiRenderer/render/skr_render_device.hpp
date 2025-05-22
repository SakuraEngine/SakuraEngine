#pragma once
#include "SkrGraphics/api.h"
#include "SkrRT/io/vram_io.hpp"
#include "SkrContainers/map.hpp"
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrBase/config.h"
#include "SkrGui/fwd_config.hpp"
#include <SDL3/SDL.h>

namespace skr::gui
{
using skr::render_graph::RenderGraph;
struct SkrRenderWindow;
struct IUpdatableImage;

enum ESkrPipelineFlag
{
    ESkrPipelineFlag_None          = 0,
    ESkrPipelineFlag_TestZ         = 1 << 0,
    ESkrPipelineFlag_Textured      = 1 << 1,
    ESkrPipelineFlag_WriteZ        = 1 << 2,
    ESkrPipelineFlag_CustomSampler = 1 << 3,
    __Count                        = 4,
};

struct SkrPipelineKey {
    ESkrPipelineFlag flags;
    ECGPUSampleCount sample_count;
    inline bool      operator==(const SkrPipelineKey& other) const
    {
        return flags == other.flags && sample_count == other.sample_count;
    }
    inline bool operator!=(const SkrPipelineKey& other) const
    {
        return flags != other.flags || sample_count != other.sample_count;
    }
    inline bool operator<(const SkrPipelineKey& other) const
    {
        return flags < other.flags || (flags == other.flags && sample_count < other.sample_count);
    }
};

} // namespace skr::gui

namespace skr
{
template <>
struct Hash<gui::SkrPipelineKey> {
    inline size_t operator()(const gui::SkrPipelineKey& key) const
    {
        const auto FlagsHash       = skr::Hash<gui::ESkrPipelineFlag>()(key.flags);
        const auto SampleCountHash = skr::Hash<ECGPUSampleCount>()(key.sample_count);
        return skr::hash_combine(FlagsHash, SampleCountHash);
    }
};
} // namespace skr

namespace skr::gui
{

struct SKR_GUI_RENDERER_API SkrRenderDevice final {

    // init & shutdown
    void init();
    void shutdown();

    // create view
    SkrRenderWindow* create_window(SDL_Window* window);
    void             destroy_window(SkrRenderWindow* view);

    // getter
    inline CGPUInstanceId         cgpu_instance() const { return _cgpu_instance; }
    inline CGPUAdapterId          cgpu_adapter() const { return _cgpu_adapter; }
    inline CGPUDeviceId           cgpu_device() const { return _cgpu_device; }
    inline CGPUQueueId            cgpu_queue() const { return _cgpu_queue; }
    inline RenderGraph*           render_graph() const { return _render_graph; }
    inline skr_io_vram_service_t* vram_service() const { return _vram_service; }

    // pipeline
    CGPURenderPipelineId get_pipeline(ESkrPipelineFlag flags, ECGPUSampleCount sample_count = CGPU_SAMPLE_COUNT_1);
    CGPURenderPipelineId create_pipeline(ESkrPipelineFlag flags, ECGPUSampleCount sample_count = CGPU_SAMPLE_COUNT_1);

private:
    // cgpu device
    CGPUInstanceId _cgpu_instance = nullptr;
    CGPUAdapterId  _cgpu_adapter  = nullptr;
    CGPUDeviceId   _cgpu_device   = nullptr;
    CGPUQueueId    _cgpu_queue    = nullptr;

    // vram
    skr_io_vram_service_t* _vram_service;

    // rg
    RenderGraph* _render_graph = nullptr;

    // PSO
    skr::Map<SkrPipelineKey, CGPURenderPipelineId> _pipelines;
    CGPURootSignaturePoolId                        _rs_pool;
    CGPUSamplerId                                  _static_color_sampler;
    CGPUVertexLayout                               _vertex_layout = {};

    // Texture updates
    Array<IUpdatableImage*> _texture_updates;
};
} // namespace skr::gui