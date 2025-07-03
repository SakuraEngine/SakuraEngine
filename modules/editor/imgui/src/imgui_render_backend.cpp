#include <SkrCore/dirty.hpp>
#include <SkrCore/log.hpp>
#include <SkrGraphics/cgpux.h>
#include <SkrRenderGraph/backend/texture_view_pool.hpp>
#include <SkrContainers/map.hpp>
#include <SkrImGui/imgui_render_backend.hpp>
#include <filesystem>
#include <SDL3/SDL.h>

// helpers
namespace skr
{
struct ImGuiRendererBackendRGViewportData {
    CGPUSurfaceId   surface                 = nullptr;
    CGPUSwapChainId swapchain               = nullptr;
    CGPUFenceId     fence                   = nullptr;
    CGPUQueueId     present_queue           = nullptr;
    uint32_t        backbuffer_index        = 0;
    ECGPULoadAction load_action             = CGPU_LOAD_ACTION_CLEAR;
    Trigger         need_acquire_next_frame = {};

    inline void destroy()
    {
        // wait rendering done
        cgpu_wait_fences(&fence, 1);
        cgpu_wait_queue_idle(present_queue);

        // destroy resources
        auto device = swapchain->device;
        cgpu_free_fence(fence);
        cgpu_free_swapchain(swapchain);
        cgpu_free_surface(device, surface);
    }
};
struct ImGuiRendererBackendRGTextureData {
    CGPUTextureId     texture      = nullptr;
    CGPUTextureViewId srv          = nullptr;
    bool              first_update = true;
};

inline static Vector<uint8_t> _read_shader_bytes(
    String       virtual_path,
    ECGPUBackend backend
)
{
    Vector<uint8_t> result;

    // combine path
    std::filesystem::path shader_path = u8"../resources/shaders/";
    shader_path /= virtual_path.c_str();
    switch (backend)
    {
    case CGPU_BACKEND_VULKAN:
        shader_path += u8".spv";
        break;
    case CGPU_BACKEND_D3D12:
    case CGPU_BACKEND_XBOX_D3D12:
        shader_path += u8".dxil";
        break;
    default:
        break;
    }

    // read file
    if (std::filesystem::exists(shader_path))
    {
        auto f = fopen(shader_path.string().c_str(), "rb");
        fseek(f, 0, SEEK_END);
        result.resize_unsafe(ftell(f));
        fseek(f, 0, SEEK_SET);
        fread(result.data(), result.size(), 1, f);
        fclose(f);
    }
    else
    {
        SKR_LOG_ERROR(u8"shader file not found: %s", shader_path.string().c_str());
    }

    return result;
}

inline static void _rebuild_swapchain(
    ImGuiViewport* vp,
    CGPUDeviceId   device,
    CGPUQueueId    present_queue,
    ImVec2         size,
    ECGPUFormat    backbuffer_format,
    uint32_t       backbuffer_count,
    bool           enable_vsync
)
{
    auto       rdata      = (ImGuiRendererBackendRGViewportData*)vp->RendererUserData;
    const auto wnd_native = vp->PlatformHandleRaw;

    // create fence & surface
    if (!rdata->fence)
    {
        rdata->fence = cgpu_create_fence(device);
    }
    if (!rdata->surface)
    {
        rdata->surface = cgpu_surface_from_native_view(
            device,
            wnd_native
        );
    }

    // wait fence
    cgpu_wait_fences(&rdata->fence, 1);
    cgpu_wait_queue_idle(present_queue);

    // destroy old swapchain
    if (rdata->swapchain)
    {
        cgpu_free_swapchain(rdata->swapchain);
    }

    // create swapchain
    CGPUSwapChainDescriptor chain_desc = {};
    chain_desc.surface                 = rdata->surface;
    chain_desc.present_queues          = &present_queue;
    chain_desc.present_queues_count    = 1;
    chain_desc.width                   = size.x;
    chain_desc.height                  = size.y;
    chain_desc.image_count             = backbuffer_count;
    chain_desc.format                  = backbuffer_format;
    chain_desc.enable_vsync            = enable_vsync;
    rdata->swapchain                   = cgpu_create_swapchain(device, &chain_desc);
    rdata->present_queue               = present_queue;
}

inline static SDL_Window* _get_sdl_wnd(ImGuiViewport* vp)
{

    auto wnd_id = (SDL_WindowID) reinterpret_cast<size_t>(vp->PlatformHandle);
    auto wnd    = SDL_GetWindowFromID(wnd_id);
    SKR_ASSERT(wnd);
    return wnd;
}

inline static void _draw_viewport(
    ImGuiViewport*                 vp,
    render_graph::RenderGraph*     render_graph,
    CGPURootSignatureId            root_sig,
    CGPURenderPipelineId           render_pipeline,
    render_graph::TextureRTVHandle back_buffer
)
{
    namespace rg = skr::render_graph;

    SkrZoneScopedN("RenderIMGUI");
    auto rdata = (ImGuiRendererBackendRGViewportData*)vp->RendererUserData;
    SKR_ASSERT(rdata != nullptr);

    // get data
    auto draw_data   = vp->DrawData;
    auto load_action = rdata->load_action;
    if (draw_data->TotalVtxCount == 0) { return; }
    uint32_t vertex_size = draw_data->TotalVtxCount * (uint32_t)sizeof(ImDrawVert);
    uint32_t index_size  = draw_data->TotalIdxCount * (uint32_t)sizeof(ImDrawIdx);

    // use CVV
    bool useCVV = true;
#if SKR_PLAT_MACOSX
    useCVV = false;
#endif

    // create or resize vb/ib
    auto vertex_buffer_handle = render_graph->create_buffer(
        [=](rg::RenderGraph& g, rg::BufferBuilder& builder) {
            SkrZoneScopedN("ConstructVBHandle");

            String name = skr::format(u8"imgui_vertices-{}", draw_data->OwnerViewport->ID);
            builder.set_name(name.c_str())
                .size(vertex_size)
                .memory_usage(useCVV ? CGPU_MEM_USAGE_CPU_TO_GPU : CGPU_MEM_USAGE_GPU_ONLY)
                .with_flags(useCVV ? CGPU_BCF_PERSISTENT_MAP_BIT : CGPU_BCF_NONE)
                .with_tags(useCVV ? kRenderGraphDynamicResourceTag : kRenderGraphDefaultResourceTag)
                .prefer_on_device()
                .as_vertex_buffer();
        }
    );
    auto index_buffer_handle = render_graph->create_buffer(
        [=](rg::RenderGraph& g, rg::BufferBuilder& builder) {
            SkrZoneScopedN("ConstructIBHandle");

            String name = skr::format(u8"imgui_indices-{}", draw_data->OwnerViewport->ID);
            builder.set_name(name.c_str())
                .size(index_size)
                .memory_usage(useCVV ? CGPU_MEM_USAGE_CPU_TO_GPU : CGPU_MEM_USAGE_GPU_ONLY)
                .with_flags(useCVV ? CGPU_BCF_PERSISTENT_MAP_BIT : CGPU_BCF_NONE)
                .with_tags(useCVV ? kRenderGraphDynamicResourceTag : kRenderGraphDefaultResourceTag)
                .prefer_on_device()
                .as_index_buffer();
        }
    );

    // upload vb/ib
    if (!useCVV)
    {
        auto upload_buffer_handle = render_graph->create_buffer(
            [=](rg::RenderGraph& g, rg::BufferBuilder& builder) {
                SkrZoneScopedN("ConstructUploadPass");

                String name = skr::format(u8"imgui_upload-{}", draw_data->OwnerViewport->ID);
                builder.set_name(name.c_str())
                    .size(index_size + vertex_size)
                    .with_tags(kRenderGraphDefaultResourceTag)
                    .as_upload_buffer();
            }
        );
        render_graph->add_copy_pass(
            [=](rg::RenderGraph& g, rg::CopyPassBuilder& builder) {
                SkrZoneScopedN("ConstructCopyPass");

                String name = skr::format(u8"imgui_copy-{}", draw_data->OwnerViewport->ID);
                builder.set_name(name.c_str())
                    .buffer_to_buffer(upload_buffer_handle.range(0, vertex_size), vertex_buffer_handle.range(0, vertex_size))
                    .buffer_to_buffer(upload_buffer_handle.range(vertex_size, vertex_size + index_size), index_buffer_handle.range(0, index_size));
            },
            [upload_buffer_handle, draw_data](rg::RenderGraph& g, rg::CopyPassContext& context) {
                auto        upload_buffer = context.resolve(upload_buffer_handle);
                ImDrawVert* vtx_dst       = (ImDrawVert*)upload_buffer->info->cpu_mapped_address;
                ImDrawIdx*  idx_dst       = (ImDrawIdx*)(vtx_dst + draw_data->TotalVtxCount);
                for (int n = 0; n < draw_data->CmdListsCount; n++)
                {
                    const ImDrawList* cmd_list = draw_data->CmdLists[n];
                    memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
                    memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
                    vtx_dst += cmd_list->VtxBuffer.Size;
                    idx_dst += cmd_list->IdxBuffer.Size;
                }
            }
        );
    }

    // cbuffer
    auto constant_buffer = render_graph->create_buffer(
        [=](rg::RenderGraph& g, rg::BufferBuilder& builder) {
            SkrZoneScopedN("ConstructCBHandle");

            String name = skr::format(u8"imgui_cbuffer-{}", draw_data->OwnerViewport->ID);
            builder.set_name(name.c_str())
                .size(sizeof(float) * 4 * 4)
                .memory_usage(CGPU_MEM_USAGE_CPU_TO_GPU)
                .with_flags(CGPU_BCF_PERSISTENT_MAP_BIT)
                .prefer_on_device()
                .as_uniform_buffer();
        }
    );

    // import textures
    rg::TextureHandle dummy_tex_handle;
    for (auto tex : ImGui::GetCurrentContext()->PlatformIO.Textures)
    {
        if (tex->Status == ImTextureStatus_OK)
        {
            dummy_tex_handle = render_graph->create_texture(
                [=](rg::RenderGraph& g, rg::TextureBuilder& builder) {
                    SkrZoneScopedN("ConstructTextureHandle");

                    auto tex_data = (ImGuiRendererBackendRGTextureData*)tex->BackendUserData;

                    String name = skr::format(u8"imgui_font-{}", tex->UniqueID);
                    builder.set_name((const char8_t*)name.c_str())
                        .import(tex_data->texture, CGPU_RESOURCE_STATE_SHADER_RESOURCE);
                }
            );
            break;
        }
    }

    // render passes
    render_graph->add_render_pass(
        [=](rg::RenderGraph& g, rg::RenderPassBuilder& builder) {
            SkrZoneScopedN("ConstructRenderPass");

            String name = skr::format(u8"imgui_render-{}", draw_data->OwnerViewport->ID);
            builder.set_name(name.c_str())
                .set_pipeline(render_pipeline)
                .read(u8"Constants", constant_buffer.range(0, sizeof(float) * 4 * 4))
                .use_buffer(vertex_buffer_handle, CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
                .use_buffer(index_buffer_handle, CGPU_RESOURCE_STATE_INDEX_BUFFER)
                .read(u8"texture0", dummy_tex_handle)
                .write(0, back_buffer, load_action);
        },
        [back_buffer, useCVV, draw_data, constant_buffer, index_buffer_handle, vertex_buffer_handle](rg::RenderGraph& g, rg::RenderPassContext& context) {
            SkrZoneScopedN("ImGuiPass");

            // get info
            const auto target_desc = g.resolve_descriptor(back_buffer);
            SKR_ASSERT(target_desc && "ImGui render target not found!");

            // upload cbuffer
            {
                float L         = draw_data->DisplayPos.x;
                float R         = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
                float T         = draw_data->DisplayPos.y;
                float B         = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
                float mvp[4][4] = {
                    { 2.0f / (R - L), 0.0f, 0.0f, 0.0f },
                    { 0.0f, 2.0f / (T - B), 0.0f, 0.0f },
                    { 0.0f, 0.0f, 0.5f, 0.0f },
                    { (R + L) / (L - R), (T + B) / (B - T), 0.5f, 1.0f },
                };
                auto buf = context.resolve(constant_buffer);
                memcpy(buf->info->cpu_mapped_address, mvp, sizeof(mvp));
            }

            // set viewport
            cgpu_render_encoder_set_viewport(context.encoder, 0.0f, 0.0f, (float)target_desc->width, (float)target_desc->height, 0.f, 1.f);

            // upload IB/VB
            auto resolved_ib = context.resolve(index_buffer_handle);
            auto resolved_vb = context.resolve(vertex_buffer_handle);
            if (useCVV)
            {
                // upload
                ImDrawVert* vtx_dst = (ImDrawVert*)resolved_vb->info->cpu_mapped_address;
                ImDrawIdx*  idx_dst = (ImDrawIdx*)resolved_ib->info->cpu_mapped_address;
                for (int n = 0; n < draw_data->CmdListsCount; n++)
                {
                    const ImDrawList* cmd_list = draw_data->CmdLists[n];
                    memcpy(
                        vtx_dst,
                        cmd_list->VtxBuffer.Data,
                        cmd_list->VtxBuffer.Size * sizeof(ImDrawVert)
                    );
                    memcpy(
                        idx_dst,
                        cmd_list->IdxBuffer.Data,
                        cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx)
                    );
                    vtx_dst += cmd_list->VtxBuffer.Size;
                    idx_dst += cmd_list->IdxBuffer.Size;
                }
            }

            // draw commands
            int          global_vtx_offset = 0;
            int          global_idx_offset = 0;
            const ImVec2 clip_off          = draw_data->DisplayPos;       // (0,0) unless using multi-viewports
            const ImVec2 clip_scale        = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)
            for (int n = 0; n < draw_data->CmdListsCount; n++)
            {
                const ImDrawList* cmd_list = draw_data->CmdLists[n];
                for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
                {
                    const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];

                    // update bind table
                    {
                        // auto imgui_tex = pcmd->TexRef;
                        // if (imgui_tex._TexData)
                        // {
                        //     auto tex_user_data = (ImGuiRendererBackendRGTextureData*)imgui_tex._TexData->BackendUserData;

                        //     CGPUDescriptorData tex_update = {};
                        //     tex_update.name               = u8"texture0";
                        //     tex_update.binding_type       = CGPU_RESOURCE_TYPE_TEXTURE;
                        //     tex_update.binding            = 0;
                        //     tex_update.textures           = &tex_user_data->srv;
                        //     tex_update.count              = 1;

                        //     cgpux_bind_table_update(
                        //         context.bind_table,
                        //         &tex_update,
                        //         1
                        //     );
                        //     cgpux_render_encoder_bind_bind_table(
                        //         context.encoder,
                        //         context.bind_table
                        //     );
                        // }
                        // else
                        // {
                        //     auto               srv        = reinterpret_cast<CGPUTextureViewId>(pcmd->TexRef._TexID);
                        //     CGPUDescriptorData tex_update = {};
                        //     tex_update.name               = u8"texture0";
                        //     tex_update.binding_type       = CGPU_RESOURCE_TYPE_TEXTURE;
                        //     tex_update.binding            = 0;
                        //     tex_update.textures           = &srv;
                        //     tex_update.count              = 1;

                        //     cgpux_bind_table_update(
                        //         context.bind_table,
                        //         &tex_update,
                        //         1
                        //     );
                        //     cgpux_render_encoder_bind_bind_table(
                        //         context.encoder,
                        //         context.bind_table
                        //     );
                        // }
                    }

                    // draw
                    if (pcmd->UserCallback != NULL)
                    {
                    }
                    else
                    {
                        // Project scissor/clipping rectangles into framebuffer space
                        ImVec4 clip_rect;
                        clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
                        clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
                        clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
                        clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;
                        if (clip_rect.x < 0.0f) clip_rect.x = 0.0f;
                        if (clip_rect.y < 0.0f) clip_rect.y = 0.0f;
                        cgpu_render_encoder_set_scissor(
                            context.encoder,
                            (uint32_t)clip_rect.x,
                            (uint32_t)clip_rect.y,
                            (uint32_t)(clip_rect.z - clip_rect.x),
                            (uint32_t)(clip_rect.w - clip_rect.y)
                        );

                        cgpu_render_encoder_bind_index_buffer(
                            context.encoder,
                            resolved_ib,
                            sizeof(uint16_t),
                            0
                        );
                        const uint32_t vert_stride = sizeof(ImDrawVert);
                        cgpu_render_encoder_bind_vertex_buffers(
                            context.encoder,
                            1,
                            &resolved_vb,
                            &vert_stride,
                            NULL
                        );
                        cgpu_render_encoder_draw_indexed(
                            context.encoder,
                            pcmd->ElemCount,
                            pcmd->IdxOffset + global_idx_offset,
                            pcmd->VtxOffset + global_vtx_offset
                        );
                    }
                }
                global_idx_offset += cmd_list->IdxBuffer.Size;
                global_vtx_offset += cmd_list->VtxBuffer.Size;
            }
        }
    );
}

} // namespace skr

// impl ImGuiRendererBackendRG
namespace skr
{
// ctor & dtor
ImGuiRendererBackendRG::ImGuiRendererBackendRG()
{
}
ImGuiRendererBackendRG::~ImGuiRendererBackendRG()
{
    if (has_init())
    {
        shutdown();
    }
}

// init & shutdown
bool ImGuiRendererBackendRG::has_init() const
{
    return _gfx_queue;
}
void ImGuiRendererBackendRG::init(const ImGuiRendererBackendRGConfig& config)
{
    SKR_ASSERT(!has_init() && "multiple init");
    SKR_ASSERT(config.ps.has_value() == config.vs.has_value() && "only one shader is setted");

    // setup draw data
    _gfx_queue         = config.queue;
    _render_graph      = config.render_graph;
    _backbuffer_count  = config.backbuffer_count;
    _backbuffer_format = config.format;

    // load shaders
    CGPUShaderEntryDescriptor ppl_shaders[2] = {};
    if (config.vs.has_value())
    {
        ppl_shaders[0] = config.vs.value();
        ppl_shaders[1] = config.ps.value();
    }
    else
    {
        auto vs_bytes = _read_shader_bytes(
            u8"imgui.vs",
            _gfx_queue->device->adapter->instance->backend
        );
        auto ps_bytes = _read_shader_bytes(
            u8"imgui.fs",
            _gfx_queue->device->adapter->instance->backend
        );

        // create lib
        CGPUShaderLibraryDescriptor vs_desc{};
        vs_desc.name      = SKR_UTF8("imgui_vertex_shader");
        vs_desc.code      = reinterpret_cast<uint32_t*>(vs_bytes.data());
        vs_desc.code_size = vs_bytes.size();
        CGPUShaderLibraryDescriptor ps_desc{};
        ps_desc.name      = SKR_UTF8("imgui_fragment_shader");
        ps_desc.code      = reinterpret_cast<uint32_t*>(ps_bytes.data());
        ps_desc.code_size = ps_bytes.size();
        auto vs_lib       = cgpu_create_shader_library(
            _gfx_queue->device,
            &vs_desc
        );
        auto fs_lib = cgpu_create_shader_library(
            _gfx_queue->device,
            &ps_desc
        );

        // fill desc
        ppl_shaders[0].library = vs_lib;
        ppl_shaders[0].stage   = CGPU_SHADER_STAGE_VERT;
        ppl_shaders[0].entry   = SKR_UTF8("vs");
        ppl_shaders[1].library = fs_lib;
        ppl_shaders[1].stage   = CGPU_SHADER_STAGE_FRAG;
        ppl_shaders[1].entry   = SKR_UTF8("fs");
    }

    // load static sampler
    CGPUSamplerId static_sampler = nullptr;
    if (config.static_sampler.has_value())
    {
        static_sampler = config.static_sampler.value();
    }
    else
    {
        CGPUSamplerDescriptor sampler_desc{};
        sampler_desc.address_u    = CGPU_ADDRESS_MODE_REPEAT;
        sampler_desc.address_v    = CGPU_ADDRESS_MODE_REPEAT;
        sampler_desc.address_w    = CGPU_ADDRESS_MODE_REPEAT;
        sampler_desc.mipmap_mode  = CGPU_MIPMAP_MODE_LINEAR;
        sampler_desc.min_filter   = CGPU_FILTER_TYPE_LINEAR;
        sampler_desc.mag_filter   = CGPU_FILTER_TYPE_LINEAR;
        sampler_desc.compare_func = CGPU_CMP_NEVER;
        static_sampler            = cgpu_create_sampler(
            _gfx_queue->device,
            &sampler_desc
        );
        _static_sampler = static_sampler;
    }

    // create root signature
    {
        const char8_t*              push_constant_name = u8"push_constants";
        const char8_t*              sampler_name       = u8"sampler0";
        CGPURootSignatureDescriptor rs_desc{};
        rs_desc.shaders              = ppl_shaders;
        rs_desc.shader_count         = 2;
        rs_desc.push_constant_names  = &push_constant_name;
        rs_desc.push_constant_count  = 1;
        rs_desc.static_sampler_names = &sampler_name;
        rs_desc.static_sampler_count = 1;
        rs_desc.static_samplers      = &static_sampler;
        _root_sig                    = cgpu_create_root_signature(
            _gfx_queue->device,
            &rs_desc
        );
    }

    // create pipeline
    {
        CGPUVertexLayout vertex_layout{};
        vertex_layout.attribute_count = 3;
        vertex_layout.attributes[0]   = {
            u8"pos",
            1,
            CGPU_FORMAT_R32G32_SFLOAT,
            0,
            0,
            sizeof(float) * 2,
            CGPU_INPUT_RATE_VERTEX
        };
        vertex_layout.attributes[1] = {
            u8"uv",
            1,
            CGPU_FORMAT_R32G32_SFLOAT,
            0,
            sizeof(float) * 2,
            sizeof(float) * 2,
            CGPU_INPUT_RATE_VERTEX
        };
        vertex_layout.attributes[2] = {
            u8"color",
            1,
            CGPU_FORMAT_R8G8B8A8_UNORM,
            0, sizeof(float) * 4,
            sizeof(uint32_t),
            CGPU_INPUT_RATE_VERTEX
        };

        CGPURasterizerStateDescriptor rs_state{};
        rs_state.cull_mode               = CGPU_CULL_MODE_NONE;
        rs_state.fill_mode               = CGPU_FILL_MODE_SOLID;
        rs_state.front_face              = CGPU_FRONT_FACE_CW;
        rs_state.slope_scaled_depth_bias = 0.f;
        rs_state.enable_depth_clamp      = false;
        rs_state.enable_scissor          = true;
        rs_state.enable_multi_sample     = false;
        rs_state.depth_bias              = 0;

        CGPUBlendStateDescriptor blend_state{};
        blend_state.blend_modes[0]       = CGPU_BLEND_MODE_ADD;
        blend_state.src_factors[0]       = CGPU_BLEND_CONST_SRC_ALPHA;
        blend_state.dst_factors[0]       = CGPU_BLEND_CONST_ONE_MINUS_SRC_ALPHA;
        blend_state.blend_alpha_modes[0] = CGPU_BLEND_MODE_ADD;
        blend_state.src_alpha_factors[0] = CGPU_BLEND_CONST_ONE;
        blend_state.dst_alpha_factors[0] = CGPU_BLEND_CONST_ONE_MINUS_SRC_ALPHA;
        blend_state.masks[0]             = CGPU_COLOR_MASK_ALL;
        blend_state.independent_blend    = false;

        CGPURenderPipelineDescriptor rp_desc{};
        rp_desc.root_signature      = _root_sig;
        rp_desc.prim_topology       = CGPU_PRIM_TOPO_TRI_LIST;
        rp_desc.vertex_layout       = &vertex_layout;
        rp_desc.vertex_shader       = &ppl_shaders[0];
        rp_desc.fragment_shader     = &ppl_shaders[1];
        rp_desc.render_target_count = 1;
        rp_desc.rasterizer_state    = &rs_state;
        rp_desc.blend_state         = &blend_state;
        rp_desc.color_formats       = &_backbuffer_format;

        _render_pipeline = cgpu_create_render_pipeline(
            _gfx_queue->device,
            &rp_desc
        );
    }

    // cleanup shader library
    if (!config.vs.has_value())
    {
        cgpu_free_shader_library(ppl_shaders[0].library);
        cgpu_free_shader_library(ppl_shaders[1].library);
    }

    // create bind table
    const char8_t*           bind_table_names[1] = { u8"texture0" };
    CGPUXBindTableDescriptor bind_table_desc     = {};
    bind_table_desc.root_signature               = _root_sig;
    bind_table_desc.names                        = bind_table_names;
    bind_table_desc.names_count                  = 1;
    _bind_table                                  = cgpux_create_bind_table(
        _render_graph->get_backend_device(),
        &bind_table_desc
    );
}
void ImGuiRendererBackendRG::shutdown()
{
    SKR_ASSERT(has_init() && "not init");

    // destroy bind table
    if (_bind_table)
    {
        cgpux_free_bind_table(_bind_table);
        _bind_table = nullptr;
    }

    // destroy render pipeline & root signature
    cgpu_free_render_pipeline(_render_pipeline);
    cgpu_free_root_signature(_root_sig);
    _render_pipeline = nullptr;
    _root_sig        = nullptr;

    // destroy sampler
    if (_static_sampler)
    {
        cgpu_free_sampler(_static_sampler);
        _static_sampler = nullptr;
    }

    // reset config
    _gfx_queue         = nullptr;
    _render_graph      = nullptr;
    _backbuffer_count  = 1;
    _backbuffer_format = CGPU_FORMAT_R8G8B8A8_UNORM;
}

// real present
void ImGuiRendererBackendRG::present_all()
{
    present_main_viewport();
    present_sub_viewports();
}
void ImGuiRendererBackendRG::present_main_viewport()
{
    auto viewport = ImGui::GetMainViewport();

    // get render data
    auto rdata = (ImGuiRendererBackendRGViewportData*)viewport->RendererUserData;
    SKR_ASSERT(rdata != nullptr);

    // do present
    CGPUQueuePresentDescriptor present_desc{};
    present_desc.index     = rdata->backbuffer_index;
    present_desc.swapchain = rdata->swapchain;
    cgpu_queue_present(rdata->present_queue, &present_desc);
}

void ImGuiRendererBackendRG::present_sub_viewports()
{
    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    for (ImGuiViewport* viewport : platform_io.Viewports)
    {
        if (viewport->Flags & ImGuiViewportFlags_IsMinimized) { continue; }
        if (viewport == ImGui::GetMainViewport()) { continue; }

        // get render data
        auto rdata = (ImGuiRendererBackendRGViewportData*)viewport->RendererUserData;
        SKR_ASSERT(rdata != nullptr);

        // do present
        CGPUQueuePresentDescriptor present_desc{};
        present_desc.index     = rdata->backbuffer_index;
        present_desc.swapchain = rdata->swapchain;
        cgpu_queue_present(rdata->present_queue, &present_desc);
    }
}

// viewport info
CGPUTextureId ImGuiRendererBackendRG::get_backbuffer(ImGuiViewport* vp)
{
    acquire_next_frame(vp);
    auto rdata = (ImGuiRendererBackendRGViewportData*)vp->RendererUserData;
    SKR_ASSERT(rdata != nullptr);
    return rdata->swapchain->back_buffers[rdata->backbuffer_index];
}
uint32_t ImGuiRendererBackendRG::get_backbuffer_index(ImGuiViewport* vp)
{
    acquire_next_frame(vp);
    auto rdata = (ImGuiRendererBackendRGViewportData*)vp->RendererUserData;
    SKR_ASSERT(rdata != nullptr);
    return rdata->backbuffer_index;
}
CGPUSwapChainId ImGuiRendererBackendRG::get_swapchain(ImGuiViewport* vp)
{
    auto rdata = (ImGuiRendererBackendRGViewportData*)vp->RendererUserData;
    SKR_ASSERT(rdata != nullptr);
    return rdata->swapchain;
}
void ImGuiRendererBackendRG::set_load_action(ImGuiViewport* vp, ECGPULoadAction load_action)
{
    auto rdata = (ImGuiRendererBackendRGViewportData*)vp->RendererUserData;
    SKR_ASSERT(rdata != nullptr);
    rdata->load_action = load_action;
}

// setup io
void ImGuiRendererBackendRG::setup_io(ImGuiIO& io)
{
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
    io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
}

// rendering utils
void ImGuiRendererBackendRG::wait_rendering_done()
{
    cgpu_wait_queue_idle(_gfx_queue);
}
void ImGuiRendererBackendRG::acquire_next_frame(ImGuiViewport* vp)
{
    auto rdata = (ImGuiRendererBackendRGViewportData*)vp->RendererUserData;
    if (rdata->need_acquire_next_frame.comsume())
    {
        cgpu_wait_fences(&rdata->fence, 1);
        CGPUAcquireNextDescriptor acquire{};
        acquire.fence            = rdata->fence;
        acquire.signal_semaphore = nullptr;
        rdata->backbuffer_index  = cgpu_acquire_next_image(rdata->swapchain, &acquire);
    }
}
void ImGuiRendererBackendRG::begin_frame()
{
    for (auto vp : ImGui::GetPlatformIO().Viewports)
    {
        auto rdata = (ImGuiRendererBackendRGViewportData*)vp->RendererUserData;
        if (rdata)
        {
            rdata->need_acquire_next_frame.trigger();
        }
    }
}
void ImGuiRendererBackendRG::end_frame()
{
}

// main window api
void ImGuiRendererBackendRG::create_main_window(ImGuiViewport* vp)
{
    create_window(vp);
}
void ImGuiRendererBackendRG::destroy_main_window(ImGuiViewport* vp)
{
    destroy_window(vp);
}
void ImGuiRendererBackendRG::resize_main_window(ImGuiViewport* vp, ImVec2 size)
{
    resize_window(vp, size);
}
void ImGuiRendererBackendRG::render_main_window(ImGuiViewport* vp)
{
    render_window(vp, nullptr);
}

// multi viewport api
void ImGuiRendererBackendRG::create_window(ImGuiViewport* vp)
{
    SKR_ASSERT(!vp->RendererUserData);
    auto wnd = _get_sdl_wnd(vp);
    int  w, h;
    SDL_GetWindowSize(wnd, &w, &h);

    vp->RendererUserData = SkrNew<ImGuiRendererBackendRGViewportData>();
    _rebuild_swapchain(
        vp,
        _render_graph->get_backend_device(),
        _render_graph->get_gfx_queue(),
        { (float)w, (float)h },
        _backbuffer_format,
        _backbuffer_count,
        false
    );
}
void ImGuiRendererBackendRG::destroy_window(ImGuiViewport* vp)
{
    SKR_ASSERT(vp->RendererUserData);
    auto rdata = (ImGuiRendererBackendRGViewportData*)vp->RendererUserData;
    rdata->destroy();
    vp->RendererUserData = nullptr;
    SkrDelete(rdata);
}
void ImGuiRendererBackendRG::resize_window(ImGuiViewport* vp, ImVec2 size)
{
    SKR_ASSERT(vp->RendererUserData);
    auto rdata = (ImGuiRendererBackendRGViewportData*)vp->RendererUserData;

    // wait rendering done
    cgpu_wait_fences(&rdata->fence, 1);

    // recreate swapchain
    _rebuild_swapchain(
        vp,
        _render_graph->get_backend_device(),
        _render_graph->get_gfx_queue(),
        size,
        _backbuffer_format,
        _backbuffer_count,
        false
    );
}
void ImGuiRendererBackendRG::render_window(ImGuiViewport* vp, void*)
{
    SKR_ASSERT(vp->RendererUserData);
    auto rdata = (ImGuiRendererBackendRGViewportData*)vp->RendererUserData;

    // wait fence
    CGPUTextureId native_backbuffer = get_backbuffer(vp);

    // import rtv
    render_graph::TextureRTVHandle back_buffer = _render_graph->create_texture(
        [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
            skr::String buf_name = skr::format(u8"imgui-window-{}", vp->ID);
            builder.set_name((const char8_t*)buf_name.c_str())
                .import(native_backbuffer, CGPU_RESOURCE_STATE_UNDEFINED)
                .allow_render_target();
        }
    );

    // draw
    _draw_viewport(
        vp,
        _render_graph,
        _root_sig,
        _render_pipeline,
        back_buffer
    );

    // present
    _render_graph->add_present_pass(
        [=](render_graph::RenderGraph& g, render_graph::PresentPassBuilder& builder) {
            skr::String pass_name = skr::format(u8"imgui-present-{}", vp->ID);
            builder.set_name((const char8_t*)pass_name.c_str())
                .swapchain(rdata->swapchain, rdata->backbuffer_index)
                .texture(back_buffer, true);
        }
    );
}

// texture api
uint32_t ImGuiRendererBackendRG::backbuffer_count() const
{
    return _backbuffer_count;
}
void ImGuiRendererBackendRG::create_texture(ImTextureData* tex_data)
{
    // create user data
    auto user_data            = SkrNew<ImGuiRendererBackendRGTextureData>();
    tex_data->BackendUserData = user_data;

    // create texture
    CGPUTextureDescriptor tex_desc = {};
    tex_desc.name                  = u8"imgui_font";
    tex_desc.width                 = static_cast<uint32_t>(tex_data->Width);
    tex_desc.height                = static_cast<uint32_t>(tex_data->Height);
    tex_desc.depth                 = 1;
    tex_desc.descriptors           = CGPU_RESOURCE_TYPE_TEXTURE;
    tex_desc.array_size            = 1;
    tex_desc.flags                 = CGPU_TCF_NONE;
    tex_desc.mip_levels            = 1;
    tex_desc.format                = CGPU_FORMAT_R8G8B8A8_UNORM;
    tex_desc.start_state           = CGPU_RESOURCE_STATE_COPY_DEST;
    tex_desc.owner_queue           = _gfx_queue;
    user_data->texture             = cgpu_create_texture(_gfx_queue->device, &tex_desc);

    // create texture view
    CGPUTextureViewDescriptor view_desc{};
    view_desc.texture           = user_data->texture;
    view_desc.base_array_layer  = 0;
    view_desc.array_layer_count = 1;
    view_desc.base_mip_level    = 0;
    view_desc.mip_level_count   = 1;
    view_desc.format            = user_data->texture->info->format;
    view_desc.aspects           = CGPU_TVA_COLOR;
    view_desc.usages            = CGPU_TVU_SRV;
    view_desc.dims              = CGPU_TEX_DIMENSION_2D;
    user_data->srv              = cgpu_create_texture_view(_gfx_queue->device, &view_desc);

    tex_data->TexID = reinterpret_cast<ImTextureID>(user_data->srv);

    // upload data
    user_data->first_update = true;
    update_texture(tex_data);
}
void ImGuiRendererBackendRG::destroy_texture(ImTextureData* tex_data)
{
    // destroy user data
    auto user_data = (ImGuiRendererBackendRGTextureData*)tex_data->BackendUserData;
    cgpu_free_texture(user_data->texture);
    cgpu_free_texture_view(user_data->srv);
    SkrDelete(user_data);

    // reset data
    tex_data->TexID           = 0;
    tex_data->BackendUserData = nullptr;
    tex_data->Status          = ImTextureStatus_Destroyed;
}
void ImGuiRendererBackendRG::update_texture(ImTextureData* tex_data)
{
    // get user data
    auto user_data = (ImGuiRendererBackendRGTextureData*)tex_data->BackendUserData;

    // create command buffer
    CGPUCommandPoolDescriptor   cmd_pool_desc = {};
    CGPUCommandBufferDescriptor cmd_desc      = {};
    auto                        cpy_cmd_pool  = cgpu_create_command_pool(
        _gfx_queue,
        &cmd_pool_desc
    );
    auto cpy_cmd = cgpu_create_command_buffer(
        cpy_cmd_pool,
        &cmd_desc
    );

    // create upload buffer
    // TODO. use updata rect
    CGPUBufferDescriptor upload_buffer_desc{};
    upload_buffer_desc.name         = u8"IMGUI_FontUploadBuffer";
    upload_buffer_desc.flags        = CGPU_BCF_PERSISTENT_MAP_BIT;
    upload_buffer_desc.descriptors  = CGPU_RESOURCE_TYPE_NONE;
    upload_buffer_desc.memory_usage = CGPU_MEM_USAGE_CPU_ONLY;
    upload_buffer_desc.size         = tex_data->GetSizeInBytes();
    CGPUBufferId tex_upload_buffer  = cgpu_create_buffer(_gfx_queue->device, &upload_buffer_desc);

    // copy data
    memcpy(
        tex_upload_buffer->info->cpu_mapped_address,
        tex_data->Pixels,
        tex_data->GetSizeInBytes()
    );

    // combine commands
    cgpu_cmd_begin(cpy_cmd);
    {
        if (!user_data->first_update)
        {
            // srv -> copy_dst
            CGPUTextureBarrier cpy_dst_barrier{};
            cpy_dst_barrier.texture   = user_data->texture;
            cpy_dst_barrier.src_state = CGPU_RESOURCE_STATE_SHADER_RESOURCE;
            cpy_dst_barrier.dst_state = CGPU_RESOURCE_STATE_COPY_DEST;
            {
                CGPUResourceBarrierDescriptor barrier_desc = {};
                barrier_desc.texture_barriers              = &cpy_dst_barrier;
                barrier_desc.texture_barriers_count        = 1;
                cgpu_cmd_resource_barrier(cpy_cmd, &barrier_desc);
            }
        }

        // copy
        CGPUBufferToTextureTransfer b2t{};
        b2t.src                              = tex_upload_buffer;
        b2t.src_offset                       = 0;
        b2t.dst                              = user_data->texture;
        b2t.dst_subresource.mip_level        = 0;
        b2t.dst_subresource.base_array_layer = 0;
        b2t.dst_subresource.layer_count      = 1;
        cgpu_cmd_transfer_buffer_to_texture(cpy_cmd, &b2t);

        // copy_dst -> srv
        CGPUTextureBarrier srv_barrier{};
        srv_barrier.texture   = user_data->texture;
        srv_barrier.src_state = CGPU_RESOURCE_STATE_COPY_DEST;
        srv_barrier.dst_state = CGPU_RESOURCE_STATE_SHADER_RESOURCE;
        {
            CGPUResourceBarrierDescriptor barrier_desc = {};
            barrier_desc.texture_barriers              = &srv_barrier;
            barrier_desc.texture_barriers_count        = 1;
            cgpu_cmd_resource_barrier(cpy_cmd, &barrier_desc);
        }
        user_data->first_update = false;
    }
    cgpu_cmd_end(cpy_cmd);

    // submit commands
    CGPUQueueSubmitDescriptor cpy_submit{};
    cpy_submit.cmds       = &cpy_cmd;
    cpy_submit.cmds_count = 1;
    cgpu_submit_queue(_gfx_queue, &cpy_submit);

    // wait for completion
    // TODO. use frame resource
    cgpu_wait_queue_idle(_gfx_queue);
    cgpu_free_command_buffer(cpy_cmd);
    cgpu_free_command_pool(cpy_cmd_pool);
    cgpu_free_buffer(tex_upload_buffer);

    tex_data->Status = ImTextureStatus_OK;
}

} // namespace skr