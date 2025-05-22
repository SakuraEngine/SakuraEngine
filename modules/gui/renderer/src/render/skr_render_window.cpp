#include "SkrGuiRenderer/render/skr_render_window.hpp"
#include "SkrGuiRenderer/render/skr_render_device.hpp"
#include "SkrGui/framework/layer/offset_layer.hpp"
#include "SkrGui/framework/layer/geometry_layer.hpp"
#include "SkrGui/backend/canvas/canvas.hpp"
#include "common/utils.h"
#include "rtm/qvvf.h"
#include "rtm/matrix3x4f.h"
#include "rtm/matrix4x4f.h"
#include "SkrProfile/profile.h"
#include "SkrGui/backend/resource/resource.hpp"
#include "SkrGuiRenderer/resource/skr_updatable_image.hpp"
#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrGui/framework/layer/native_window_layer.hpp"
#include "SkrBase/math.h"

namespace skr::gui
{
SkrRenderWindow::SkrRenderWindow(SkrRenderDevice* owner, SDL_Window* window)
    : _owner(owner)
    , _window(window)
{
    auto device = _owner->cgpu_device();
    auto queue  = _owner->cgpu_queue();

    _cgpu_fence   = cgpu_create_fence(_owner->cgpu_device());
    _cgpu_surface = cgpu_surface_from_native_view(device, SDLGetNativeWindowHandle(_window));

    int32_t width, height;
    SDL_GetWindowSize(_window, &width, &height);
    CGPUSwapChainDescriptor chain_desc = {};
    chain_desc.surface                 = _cgpu_surface;
    chain_desc.present_queues          = &queue;
    chain_desc.present_queues_count    = 1;
    chain_desc.width                   = width;
    chain_desc.height                  = height;
    chain_desc.image_count             = 2;
    chain_desc.format                  = CGPU_FORMAT_B8G8R8A8_UNORM; // TODO: use correct screen buffer format
    chain_desc.enable_vsync            = false;
    _cgpu_swapchain                    = cgpu_create_swapchain(device, &chain_desc);
}
SkrRenderWindow::~SkrRenderWindow()
{
    if (_cgpu_fence)
    {
        cgpu_free_fence(_cgpu_fence);
    }
    if (_cgpu_surface)
    {
        cgpu_free_surface(_owner->cgpu_device(), _cgpu_surface);
    }
    if (_cgpu_swapchain)
    {
        cgpu_free_swapchain(_cgpu_swapchain);
    }
}

void SkrRenderWindow::sync_window_size()
{
    auto device = _owner->cgpu_device();
    auto queue  = _owner->cgpu_queue();

    // TODO. fix this hack
    cgpu_wait_queue_idle(queue);

    if (!_cgpu_fence)
    {
        _cgpu_fence = cgpu_create_fence(_owner->cgpu_device());
    }
    if (!_cgpu_surface)
    {
        _cgpu_surface = cgpu_surface_from_native_view(device, SDLGetNativeWindowHandle(_window));
    }

    if (_cgpu_fence)
    {
        cgpu_wait_fences(&_cgpu_fence, 1);
    }
    if (_cgpu_swapchain)
    {
        cgpu_free_swapchain(_cgpu_swapchain);
    }

    int32_t width, height;
    SDL_GetWindowSize(_window, &width, &height);
    CGPUSwapChainDescriptor chain_desc = {};
    chain_desc.surface                 = _cgpu_surface;
    chain_desc.present_queues          = &queue;
    chain_desc.present_queues_count    = 1;
    chain_desc.width                   = width;
    chain_desc.height                  = height;
    chain_desc.image_count             = 2;
    chain_desc.format                  = CGPU_FORMAT_B8G8R8A8_UNORM; // TODO: use correct screen buffer format
    chain_desc.enable_vsync            = false;
    _cgpu_swapchain                    = cgpu_create_swapchain(device, &chain_desc);
}

void SkrRenderWindow::render(const NativeWindowLayer* layer, Sizef window_size)
{
    // step1. prepare draw data
    _prepare_draw_data(layer, window_size);

    // step2. upload draw data
    _upload_draw_data();

    // step3. declare render resource
    _declare_render_resources();

    // step4. render
    _render();
}
void SkrRenderWindow::present()
{
    CGPUQueuePresentDescriptor present_desc = {};
    present_desc.index                      = _backbuffer_index;
    present_desc.swapchain                  = _cgpu_swapchain;
    cgpu_queue_present(_owner->cgpu_queue(), &present_desc);
}

void SkrRenderWindow::_prepare_draw_data(const NativeWindowLayer* layer, Sizef window_size)
{
    // cleanup data
    _vertices.clear();
    _indices.clear();
    _commands.clear();
    _transforms.clear();
    _projections.clear();
    _render_data.clear();

    // copy data
    auto canvas = layer->children().data()[0]->type_cast_fast<GeometryLayer>()->canvas();
    _vertices.assign(canvas->vertices().data(), canvas->vertices().size());
    _indices.assign(canvas->indices().data(), canvas->indices().size());
    for (const auto& cmd : canvas->commands())
    {
        // copy command
        DrawCommand draw_cmd     = {};
        draw_cmd.texture         = cmd.texture;
        draw_cmd.index_begin     = cmd.index_begin;
        draw_cmd.index_count     = cmd.index_count;
        draw_cmd.texture_swizzle = cmd.texture_swizzle;

        // make transform
        auto tb_cursor = _transforms.size();
        {
            auto&      transform = _transforms.add_default().ref();
            TransformF t;
            t.scale    = { 1.f, 1.f, 1.f };
            t.position = { 0.f, 0.f, 0.f };
            t.rotation = QuatF(RotatorF(0.f, 0.f, 0));
            transform  = RtmConvert<float4x4>::to_rtm(t);
        }

        // make projection
        auto               pb_cursor    = _projections.size();
        auto&              projection   = _projections.add_default().ref();
        const skr_float2_t zero_point   = { window_size.width * 0.5f, window_size.height * 0.5f };
        const skr_float2_t eye_position = { zero_point.x, zero_point.y };
        rtm::matrix3x4f    view         = rtm::matrix_cast(rtm::view_look_to(
            rtm::vector_set(eye_position.x, eye_position.y, -1.f, 0.0f),
            rtm::vector_set(0.f, 0.f, 1.f, 0.0f),
            rtm::vector_set(0.0f, 1.0f, 0.0f, 0.0f)
        ));
        // flip y axis for direct x
        view = rtm::matrix_mul(view, (rtm::matrix3x4f)rtm::matrix_from_scale(rtm::vector_set(1.f, -1.f, 1.f, 0.f)));

        const auto proj = rtm::proj_orthographic(
            window_size.width,
            window_size.height,
            0.01f,
            1000.f
        );
        projection = rtm::matrix_mul(rtm::matrix_cast(view), proj);

        // make render data
        auto  rb_cursor        = _render_data.size();
        auto& render_data      = _render_data.add_default().ref();
        render_data.rows[0][0] = static_cast<float>(cmd.texture_swizzle.r);
        render_data.rows[0][1] = static_cast<float>(cmd.texture_swizzle.g);
        render_data.rows[0][2] = static_cast<float>(cmd.texture_swizzle.b);
        render_data.rows[0][3] = static_cast<float>(cmd.texture_swizzle.a);

        // record buffer info
        draw_cmd.transform_buffer_offset   = tb_cursor * sizeof(rtm::matrix4x4f);
        draw_cmd.projection_buffer_offset  = pb_cursor * sizeof(rtm::matrix4x4f);
        draw_cmd.render_data_buffer_offset = rb_cursor * sizeof(skr_float4x4_t);

        // flag
        if (draw_cmd.texture && draw_cmd.texture->state() == EResourceState::Okey)
        {
            draw_cmd.pipeline_flags = ESkrPipelineFlag(draw_cmd.pipeline_flags | ESkrPipelineFlag_Textured);
        }

        _commands.add(draw_cmd);
    }
}
void SkrRenderWindow::_upload_draw_data()
{
    const uint64_t vertices_size    = _vertices.size() * sizeof(PaintVertex);
    const uint64_t indices_size     = _indices.size() * sizeof(PaintIndex);
    const uint64_t transform_size   = _transforms.size() * sizeof(rtm::matrix4x4f);
    const uint64_t projection_size  = _projections.size() * sizeof(rtm::matrix4x4f);
    const uint64_t render_data_size = _render_data.size() * sizeof(skr_float4x4_t);
    const bool     useCVV           = false;

    auto rg = _owner->render_graph();

    auto vertex_buffer = rg->create_buffer(
        [=](render_graph::RenderGraph& g, render_graph::BufferBuilder& builder) {
            builder.set_name(u8"gui_vertex_buffer")
                .size(vertices_size)
                .memory_usage(useCVV ? CGPU_MEM_USAGE_CPU_TO_GPU : CGPU_MEM_USAGE_GPU_ONLY)
                .with_flags(useCVV ? CGPU_BCF_PERSISTENT_MAP_BIT : CGPU_BCF_NONE)
                .with_tags(useCVV ? kRenderGraphDynamicResourceTag : kRenderGraphDefaultResourceTag)
                .prefer_on_device()
                .as_vertex_buffer();
        }
    );
    auto index_buffer = rg->create_buffer(
        [=](render_graph::RenderGraph& g, render_graph::BufferBuilder& builder) {
            builder.set_name(u8"gui_index_buffer")
                .size(indices_size)
                .memory_usage(useCVV ? CGPU_MEM_USAGE_CPU_TO_GPU : CGPU_MEM_USAGE_GPU_ONLY)
                .with_flags(useCVV ? CGPU_BCF_PERSISTENT_MAP_BIT : CGPU_BCF_NONE)
                .with_tags(useCVV ? kRenderGraphDynamicResourceTag : kRenderGraphDefaultResourceTag)
                .prefer_on_device()
                .as_index_buffer();
        }
    );
    auto transform_buffer = rg->create_buffer(
        [=](render_graph::RenderGraph& g, render_graph::BufferBuilder& builder) {
            builder.set_name(u8"gui_transform_buffer")
                .size(transform_size)
                .memory_usage(useCVV ? CGPU_MEM_USAGE_CPU_TO_GPU : CGPU_MEM_USAGE_GPU_ONLY)
                .with_flags(useCVV ? CGPU_BCF_PERSISTENT_MAP_BIT : CGPU_BCF_NONE)
                .with_tags(useCVV ? kRenderGraphDynamicResourceTag : kRenderGraphDefaultResourceTag)
                .prefer_on_device()
                .as_vertex_buffer();
        }
    );
    auto projection_buffer = rg->create_buffer(
        [=](render_graph::RenderGraph& g, render_graph::BufferBuilder& builder) {
            builder.set_name(u8"gui_projection_buffer")
                .size(projection_size)
                .memory_usage(useCVV ? CGPU_MEM_USAGE_CPU_TO_GPU : CGPU_MEM_USAGE_GPU_ONLY)
                .with_flags(useCVV ? CGPU_BCF_PERSISTENT_MAP_BIT : CGPU_BCF_NONE)
                .with_tags(useCVV ? kRenderGraphDynamicResourceTag : kRenderGraphDefaultResourceTag)
                .prefer_on_device()
                .as_vertex_buffer();
        }
    );
    auto rdata_buffer = rg->create_buffer(
        [=](render_graph::RenderGraph& g, render_graph::BufferBuilder& builder) {
            builder.set_name(u8"gui_rdata_buffer")
                .size(render_data_size)
                .memory_usage(useCVV ? CGPU_MEM_USAGE_CPU_TO_GPU : CGPU_MEM_USAGE_GPU_ONLY)
                .with_flags(useCVV ? CGPU_BCF_PERSISTENT_MAP_BIT : CGPU_BCF_NONE)
                .with_tags(useCVV ? kRenderGraphDynamicResourceTag : kRenderGraphDefaultResourceTag)
                .prefer_on_device()
                .as_vertex_buffer();
        }
    );

    _vertex_buffer      = vertex_buffer;
    _index_buffer       = index_buffer;
    _transform_buffer   = transform_buffer;
    _projection_buffer  = projection_buffer;
    _render_data_buffer = rdata_buffer;

    if (!useCVV)
    {
        auto upload_buffer_handle = rg->create_buffer(
            [=](render_graph::RenderGraph& g, render_graph::BufferBuilder& builder) {
                SkrZoneScopedN("ConstructUploadPass");
                builder.set_name(u8"gui_upload_buffer")
                    .size(indices_size + vertices_size + transform_size + projection_size + render_data_size)
                    .with_tags(kRenderGraphDynamicResourceTag)
                    .as_upload_buffer();
            }
        );
        rg->add_copy_pass(
            [=](render_graph::RenderGraph& g, render_graph::CopyPassBuilder& builder) {
                SkrZoneScopedN("ConstructCopyPass");
                builder.set_name(u8"gui_copy_pass");
                uint64_t cursor = 0;
                builder.buffer_to_buffer(upload_buffer_handle.range(cursor, vertices_size), vertex_buffer.range(0, vertices_size));
                cursor += vertices_size;
                builder.buffer_to_buffer(upload_buffer_handle.range(cursor, cursor + indices_size), index_buffer.range(0, indices_size));
                cursor += indices_size;
                builder.buffer_to_buffer(upload_buffer_handle.range(cursor, cursor + transform_size), transform_buffer.range(0, transform_size));
                cursor += transform_size;
                builder.buffer_to_buffer(upload_buffer_handle.range(cursor, cursor + projection_size), projection_buffer.range(0, projection_size));
                cursor += projection_size;
                builder.buffer_to_buffer(upload_buffer_handle.range(cursor, cursor + render_data_size), rdata_buffer.range(0, render_data_size));
            },
            [upload_buffer_handle, this](render_graph::RenderGraph& g, render_graph::CopyPassContext& context) {
                auto           upload_buffer     = context.resolve(upload_buffer_handle);
                const uint64_t vertices_count    = _vertices.size();
                const uint64_t indices_count     = _indices.size();
                const uint64_t transforms_count  = _transforms.size();
                const uint64_t projections_count = _projections.size();
                const uint64_t render_data_count = _render_data.size();

                PaintVertex*     vtx_dst        = (PaintVertex*)upload_buffer->info->cpu_mapped_address;
                PaintIndex*      idx_dst        = (PaintIndex*)(vtx_dst + vertices_count);
                rtm::matrix4x4f* transform_dst  = (rtm::matrix4x4f*)(idx_dst + indices_count);
                rtm::matrix4x4f* projection_dst = (rtm::matrix4x4f*)(transform_dst + transforms_count);
                skr_float4x4_t*  rdata_dst      = (skr_float4x4_t*)(projection_dst + projections_count);

                const skr::span<PaintVertex>     render_vertices    = _vertices;
                const skr::span<PaintIndex>      render_indices     = _indices;
                const skr::span<rtm::matrix4x4f> render_transforms  = _transforms;
                const skr::span<rtm::matrix4x4f> render_projections = _projections;
                const skr::span<skr_float4x4_t>  render_data        = _render_data;

                memcpy(vtx_dst, render_vertices.data(), vertices_count * sizeof(PaintVertex));
                memcpy(idx_dst, render_indices.data(), indices_count * sizeof(PaintIndex));
                memcpy(transform_dst, render_transforms.data(), transforms_count * sizeof(rtm::matrix4x4f));
                memcpy(projection_dst, render_projections.data(), projections_count * sizeof(rtm::matrix4x4f));
                memcpy(rdata_dst, render_data.data(), render_data_count * sizeof(skr_float4x4_t));
            }
        );
    }
}
void SkrRenderWindow::_declare_render_resources()
{
    // acquire frame
    {
        SkrZoneScopedN("WaitPresent");
        cgpu_wait_fences(&_cgpu_fence, 1);
    }
    {
        SkrZoneScopedN("AcquireFrame");
        auto acquire_desc  = make_zeroed<CGPUAcquireNextDescriptor>();
        acquire_desc.fence = _cgpu_fence;
        _backbuffer_index  = cgpu_acquire_next_image(_cgpu_swapchain, &acquire_desc);
    }

    // declare resources
    const auto    sample_count        = CGPU_SAMPLE_COUNT_1; // TODO. sample count
    CGPUTextureId imported_backbuffer = _cgpu_swapchain->back_buffers[_backbuffer_index];
    auto          graph               = _owner->render_graph();
    if (sample_count != CGPU_SAMPLE_COUNT_1)
    {
        _back_buffer = graph->create_texture(
            [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
                builder.set_name(SKR_UTF8("presentbuffer"))
                    .import(imported_backbuffer, CGPU_RESOURCE_STATE_PRESENT)
                    .allow_render_target();
            }
        );
        const auto back_desc  = graph->resolve_descriptor(_back_buffer);
        auto       msaaTarget = graph->create_texture(
            [=](skr::render_graph::RenderGraph& g, skr::render_graph::TextureBuilder& builder) {
                builder.set_name(SKR_UTF8("backbuffer"))
                    .extent(back_desc->width, back_desc->height)
                    .format(back_desc->format)
                    .sample_count(sample_count)
                    .allow_render_target();
                if (back_desc->width > 2048) builder.allocate_dedicated();
            }
        );
        (void)msaaTarget;
    }
    else
    {
        _back_buffer = graph->create_texture(
            [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
                builder.set_name(SKR_UTF8("backbuffer"))
                    .import(imported_backbuffer, CGPU_RESOURCE_STATE_PRESENT)
                    .allow_render_target();
            }
        );
    }
    _depth_buffer = graph->create_texture(
        [=, this](skr::render_graph::RenderGraph& g, skr::render_graph::TextureBuilder& builder) {
            const auto texInfo = _cgpu_swapchain->back_buffers[0]->info;
            builder.set_name(SKR_UTF8("depth"))
                .extent(texInfo->width, texInfo->height)
                .format(CGPU_FORMAT_D32_SFLOAT)
                .sample_count(sample_count)
                .allow_depth_stencil();
            if (texInfo->width > 2048) builder.allocate_dedicated();
        }
    );
}
void SkrRenderWindow::_render()
{
    auto rg     = _owner->render_graph();
    auto target = _back_buffer;
    auto depth  = _depth_buffer;

    // TODO. multi pass
    rg->add_render_pass(
        [&](render_graph::RenderGraph& g, render_graph::RenderPassBuilder& builder) {
        SkrZoneScopedN("ConstructRenderPass");
        const auto back_desc = g.resolve_descriptor(target);
        builder.set_name(u8"gui_render_pass")
            .use_buffer(_vertex_buffer, CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
            .use_buffer(_transform_buffer, CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
            .use_buffer(_projection_buffer, CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
            .use_buffer(_render_data_buffer, CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
            .use_buffer(_index_buffer, CGPU_RESOURCE_STATE_INDEX_BUFFER)
            .set_depth_stencil(depth.clear_depth(1.f))
            .write(0, target, CGPU_LOAD_ACTION_CLEAR);
        if (back_desc->sample_count > 1)
        {
            skr::render_graph::TextureHandle real_target = rg->get_texture(u8"presentbuffer");
            builder.resolve_msaa(0, real_target);
        } },
        [this, target](render_graph::RenderGraph& g, render_graph::RenderPassContext& ctx) {
            SkrZoneScopedN("GUI-RenderPass");
            const auto     target_desc              = g.resolve_descriptor(target);
            auto           resolved_ib              = ctx.resolve(_index_buffer);
            auto           resolved_vb              = ctx.resolve(_vertex_buffer);
            auto           resolved_tb              = ctx.resolve(_transform_buffer);
            auto           resolved_pb              = ctx.resolve(_projection_buffer);
            auto           resolved_rdata           = ctx.resolve(_render_data_buffer);
            CGPUBufferId   vertex_streams[4]        = { resolved_vb, resolved_tb, resolved_pb, resolved_rdata };
            const uint32_t vertex_stream_strides[4] = { sizeof(PaintVertex), sizeof(rtm::matrix4x4f), sizeof(rtm::matrix4x4f), sizeof(skr_float4x4_t) };

            cgpu_render_encoder_set_viewport(ctx.encoder, 0.0f, 0.0f, (float)target_desc->width, (float)target_desc->height, 0.f, 1.f);
            cgpu_render_encoder_set_scissor(ctx.encoder, 0, 0, (uint32_t)target_desc->width, (uint32_t)target_desc->height);

            SkrPipelineKey pipeline_key_cache = { ESkrPipelineFlag::__Count, CGPU_SAMPLE_COUNT_1 };

            for (const auto& cmd : _commands)
            {
                const bool     use_texture = cmd.texture && cmd.texture->state() == skr::gui::EResourceState::Okey;
                SkrPipelineKey key         = { cmd.pipeline_flags, target_desc->sample_count };
                if (pipeline_key_cache != key)
                {
                    CGPURenderPipelineId this_pipeline = _owner->get_pipeline(key.flags, key.sample_count);
                    cgpu_render_encoder_bind_pipeline(ctx.encoder, this_pipeline);
                    pipeline_key_cache = key;
                }

                if (use_texture)
                {
                    const auto gui_texture = cmd.texture->type_cast<SkrUpdatableImage>();
                    cgpux_render_encoder_bind_bind_table(ctx.encoder, gui_texture->bind_table());
                }

                const uint32_t vertex_stream_offsets[4] = {
                    (uint32_t)cmd.vertex_buffer_offset,
                    (uint32_t)cmd.transform_buffer_offset,
                    (uint32_t)cmd.projection_buffer_offset,
                    (uint32_t)cmd.render_data_buffer_offset
                };
                cgpu_render_encoder_bind_index_buffer(ctx.encoder, resolved_ib, sizeof(PaintIndex), cmd.index_buffer_offset);
                cgpu_render_encoder_bind_vertex_buffers(ctx.encoder, 4, vertex_streams, vertex_stream_strides, vertex_stream_offsets);
                cgpu_render_encoder_draw_indexed_instanced(ctx.encoder, (uint32_t)cmd.index_count, (uint32_t)cmd.index_begin, 1, 0, 0);
            }
        }
    );

    // present pass
    rg->add_present_pass(
        [=, this](render_graph::RenderGraph& g, render_graph::PresentPassBuilder& builder) {
            builder.set_name(SKR_UTF8("present"))
                .swapchain(_cgpu_swapchain, _backbuffer_index)
                .texture(_back_buffer, true);
        }
    );
}

} // namespace skr::gui