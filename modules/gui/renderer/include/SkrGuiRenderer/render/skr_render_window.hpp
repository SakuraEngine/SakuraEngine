#pragma once
#include "SkrBase/config.h"
#include "SkrGraphics/api.h"
#include "SkrGui/backend/canvas/canvas_types.hpp"
#include "rtm/camera_utilsf.h"
#include "SkrRenderGraph/frontend/render_graph.hpp"
#include "SkrGuiRenderer/render/skr_render_device.hpp"
#include <SDL3/SDL.h>

namespace skr::gui
{
struct SkrRenderDevice;
struct NativeWindowLayer;

struct SKR_GUI_RENDERER_API SkrRenderWindow final {
    SkrRenderWindow(SkrRenderDevice* owner, SDL_Window* window);
    ~SkrRenderWindow();

    void sync_window_size();
    void render(const NativeWindowLayer* layer, Sizef window_size);
    void present();

private:
    void _prepare_draw_data(const NativeWindowLayer* layer, Sizef window_size);
    void _upload_draw_data();
    // void _update_textures(); handled by SkrRenderDevice
    void _declare_render_resources();
    void _render();

private:
    // owner
    SkrRenderDevice* _owner  = nullptr;
    SDL_Window*      _window = nullptr;

    // cgpu data
    CGPUSurfaceId               _cgpu_surface     = nullptr;
    CGPUSwapChainId             _cgpu_swapchain   = nullptr;
    CGPUFenceId                 _cgpu_fence       = nullptr;
    uint32_t                    _backbuffer_index = 0;
    render_graph::TextureHandle _back_buffer      = {};
    render_graph::TextureHandle _depth_buffer     = {};

    // draw data
    Array<PaintVertex>     _vertices;
    Array<PaintIndex>      _indices;
    Array<rtm::matrix4x4f> _transforms;
    Array<rtm::matrix4x4f> _projections;
    // texture_swizzle:[_, _, _, _] None-0, R-1, G-2, B-3, A-4
    // placeholder0:   [_, _, _, _]
    // placeholder1:   [_, _, _, _]
    // placeholder2:   [_, _, _, _]
    Array<skr_float4x4_t> _render_data;
    struct DrawCommand {
        // CPU data
        IImage* texture         = nullptr;
        size_t  index_begin     = 0;
        size_t  index_count     = 0;
        Swizzle texture_swizzle = {};

        // GPU data
        size_t index_buffer_offset       = 0;
        size_t vertex_buffer_offset      = 0;
        size_t transform_buffer_offset   = 0;
        size_t projection_buffer_offset  = 0;
        size_t render_data_buffer_offset = 0;

        // features
        ESkrPipelineFlag pipeline_flags = ESkrPipelineFlag_None;
    };
    Array<DrawCommand> _commands;

    // buffer
    skr::render_graph::BufferHandle _vertex_buffer      = {};
    skr::render_graph::BufferHandle _index_buffer       = {};
    skr::render_graph::BufferHandle _transform_buffer   = {};
    skr::render_graph::BufferHandle _projection_buffer  = {};
    skr::render_graph::BufferHandle _render_data_buffer = {};
};
} // namespace skr::gui