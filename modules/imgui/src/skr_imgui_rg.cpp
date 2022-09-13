#include <EASTL/string.h>
#include "platform/configure.h"
#include "utils./log.h"
#include "imgui/imgui.h"
#include "imgui/skr_imgui.h"
#include "imgui/skr_imgui_rg.h"

namespace skr::imgui
{
SKR_IMGUI_API CGPUTextureId font_texture;
SKR_IMGUI_API CGPURootSignatureId root_sig;
SKR_IMGUI_API CGPURenderPipelineId render_pipeline;
SKR_IMGUI_API skr::render_graph::TextureHandle font_handle;
SKR_IMGUI_API skr::render_graph::BufferHandle vertex_buffer_handle;
SKR_IMGUI_API skr::render_graph::BufferHandle index_buffer_handle;
SKR_IMGUI_API skr::render_graph::BufferHandle upload_buffer_handle;

namespace rg = skr::render_graph;

void imgui_create_fonts(CGPUQueueId queue);
void imgui_create_pipeline(const RenderGraphImGuiDescriptor* desc);
void imguir_create_window(ImGuiViewport* viewport);
void imguir_destroy_window(ImGuiViewport* viewport);
void imguir_resize_window(ImGuiViewport* viewport, ImVec2 size);
void imguir_render_window(ImGuiViewport* viewport, void*);
void imguir_swap_buffers(ImGuiViewport* viewport, void*);
}


using namespace skr::imgui;
SKR_IMGUI_API void render_graph_imgui_initialize(const RenderGraphImGuiDescriptor* desc)
{
    ImGuiIO& io = ImGui::GetIO();
    io.BackendRendererName = "imgui_render_graph";
    // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
    io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;

    // Our render function expect RendererUserData to be storing the window render buffer we need (for the main viewport we won't use ->Window)
    ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    (void)main_viewport;

    imgui_create_fonts(desc->queue);
    imgui_create_pipeline(desc);

    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    platform_io.Renderer_CreateWindow = &imguir_create_window;
    platform_io.Renderer_DestroyWindow = &imguir_destroy_window;
    platform_io.Renderer_SetWindowSize = &imguir_resize_window;
    platform_io.Renderer_RenderWindow = &imguir_render_window;
    platform_io.Renderer_SwapBuffers = &imguir_swap_buffers;
}

void imguir_render_draw_data(ImDrawData* draw_data, 
    skr::render_graph::RenderGraph* render_graph, skr::render_graph::TextureRTVHandle target, ECGPULoadAction load_action)
{
    bool useCVV = true;
#ifdef SKR_OS_MACOSX
    useCVV = false;
#endif
    if (draw_data->TotalVtxCount > 0)
    {
        // create or resize the vertex/index buffers
        auto device = render_graph->get_backend_device();
        size_t vertex_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
        size_t index_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);
        font_handle = render_graph->create_texture(
            [=](rg::RenderGraph& g, rg::TextureBuilder& builder) {
                builder.set_name("imgui_font_texture")
                    .import(font_texture, CGPU_RESOURCE_STATE_SHADER_RESOURCE);
            });
        // vb & ib
        vertex_buffer_handle = render_graph->create_buffer(
            [=](rg::RenderGraph& g, rg::BufferBuilder& builder) {
                builder.set_name("imgui_vertex_buffer")
                    .size(vertex_size)
                    .memory_usage(useCVV ? CGPU_MEM_USAGE_CPU_TO_GPU : CGPU_MEM_USAGE_GPU_ONLY)
                    .with_flags(useCVV ? CGPU_BCF_PERSISTENT_MAP_BIT : CGPU_BCF_NONE)
                    .with_tags(useCVV ? kRenderGraphDynamicResourceTag : kRenderGraphDefaultResourceTag)
                    .prefer_on_device()
                    .as_vertex_buffer();
            });
        index_buffer_handle = render_graph->create_buffer(
            [=](rg::RenderGraph& g, rg::BufferBuilder& builder) {
                builder.set_name("imgui_index_buffer")
                    .size(index_size)
                    .memory_usage(useCVV ? CGPU_MEM_USAGE_CPU_TO_GPU : CGPU_MEM_USAGE_GPU_ONLY)
                    .with_flags(useCVV ? CGPU_BCF_PERSISTENT_MAP_BIT : CGPU_BCF_NONE)
                    .with_tags(useCVV ? kRenderGraphDynamicResourceTag : kRenderGraphDefaultResourceTag)
                    .prefer_on_device()
                    .as_index_buffer();
            });
        if (!useCVV)
        {
            upload_buffer_handle = render_graph->create_buffer(
                [=](rg::RenderGraph& g, rg::BufferBuilder& builder) {
                    builder.set_name("imgui_upload_buffer")
                        .size(index_size + vertex_size)
                        .with_tags(kRenderGraphDefaultResourceTag)
                        .as_upload_buffer();
                });
            render_graph->add_copy_pass(
                [=](rg::RenderGraph& g, rg::CopyPassBuilder& builder) {
                    builder.set_name("imgui_geom_transfer")
                        .buffer_to_buffer(upload_buffer_handle.range(0, vertex_size), vertex_buffer_handle.range(0, vertex_size))
                        .buffer_to_buffer(upload_buffer_handle.range(vertex_size, vertex_size + index_size), index_buffer_handle.range(0, index_size));
                },
                [](rg::RenderGraph& g, rg::CopyPassContext& context){
                    auto upload_buffer = context.resolve(upload_buffer_handle);
                    ImDrawData* draw_data = ImGui::GetDrawData();
                    ImDrawVert* vtx_dst = (ImDrawVert*)upload_buffer->cpu_mapped_address;
                    ImDrawIdx* idx_dst = (ImDrawIdx*)(vtx_dst + draw_data->TotalVtxCount);
                    for (int n = 0; n < draw_data->CmdListsCount; n++)
                    {
                        const ImDrawList* cmd_list = draw_data->CmdLists[n];
                        memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
                        memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
                        vtx_dst += cmd_list->VtxBuffer.Size;
                        idx_dst += cmd_list->IdxBuffer.Size;
                    }
                });
        }
        auto constant_buffer = render_graph->create_buffer(
            [=](rg::RenderGraph& g, rg::BufferBuilder& builder) {
                builder.set_name("imgui_cbuffer")
                    .size(sizeof(float) * 4 * 4)
                    .with_tags(kRenderGraphDynamicResourceTag)
                    .with_flags(CGPU_BCF_PERSISTENT_MAP_BIT)
                    .prefer_on_device()
                    .as_uniform_buffer();
            });
        // add pass
        render_graph->add_render_pass([=](rg::RenderGraph& g, rg::RenderPassBuilder& builder) {
            builder.set_name("imgui_pass")
                .set_pipeline(render_pipeline)
                .read("Constants", constant_buffer.range(0, sizeof(float) * 4 * 4))
                .use_buffer(vertex_buffer_handle, CGPU_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
                .use_buffer(index_buffer_handle, CGPU_RESOURCE_STATE_INDEX_BUFFER)
                .read("texture0", font_handle)
                .write(0, target, load_action);
        },
        [target, useCVV, constant_buffer](rg::RenderGraph& g, rg::RenderPassContext& context) {
            auto target_node = g.resolve(target);
            const auto& target_desc = target_node->get_desc();
            ImDrawData* draw_data = ImGui::GetDrawData();
            {
                float L = draw_data->DisplayPos.x;
                float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
                float T = draw_data->DisplayPos.y;
                float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
                float mvp[4][4] =
                {
                    { 2.0f/(R-L),   0.0f,           0.0f,       0.0f },
                    { 0.0f,         2.0f/(T-B),     0.0f,       0.0f },
                    { 0.0f,         0.0f,           0.5f,       0.0f },
                    { (R+L)/(L-R),  (T+B)/(B-T),    0.5f,       1.0f },
                };
                auto buf = context.resolve(constant_buffer);
                memcpy(buf->cpu_mapped_address, mvp, sizeof(mvp));
            }
            cgpu_render_encoder_set_viewport(context.encoder,
                0.0f, 0.0f,
                (float)target_desc.width,
                (float)target_desc.height,
                0.f, 1.f);
            // drawcalls
            // Will project scissor/clipping rectangles into framebuffer space
            const ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
            const ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)
            // Render command lists
            // (Because we merged all buffers into a single one, we maintain our own offset into them)
            int global_vtx_offset = 0;
            int global_idx_offset = 0;
            for (int n = 0; n < draw_data->CmdListsCount; n++)
            {
                const ImDrawList* cmd_list = draw_data->CmdLists[n];
                for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
                {
                    const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
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
                        cgpu_render_encoder_set_scissor(context.encoder,
                            (uint32_t)clip_rect.x, (uint32_t)clip_rect.y,
                            (uint32_t)(clip_rect.z - clip_rect.x),
                            (uint32_t)(clip_rect.w - clip_rect.y));

                        auto resolved_ib = context.resolve(index_buffer_handle);
                        auto resolved_vb = context.resolve(vertex_buffer_handle);
                        if (useCVV)
                        {
                            // upload
                            ImDrawVert* vtx_dst = (ImDrawVert*)resolved_vb->cpu_mapped_address;
                            ImDrawIdx* idx_dst = (ImDrawIdx*)resolved_ib->cpu_mapped_address;
                            for (int n = 0; n < draw_data->CmdListsCount; n++)
                            {
                                const ImDrawList* cmd_list = draw_data->CmdLists[n];
                                memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
                                memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
                                vtx_dst += cmd_list->VtxBuffer.Size;
                                idx_dst += cmd_list->IdxBuffer.Size;
                            }
                        }
                        cgpu_render_encoder_bind_index_buffer(context.encoder,
                            resolved_ib, sizeof(uint16_t), 0);
                            const uint32_t stride = sizeof(ImDrawVert);
                        cgpu_render_encoder_bind_vertex_buffers(context.encoder,
                            1, &resolved_vb, &stride, NULL);
                            cgpu_render_encoder_draw_indexed(context.encoder,
                            pcmd->ElemCount,
                            pcmd->IdxOffset + global_idx_offset,
                            pcmd->VtxOffset + global_vtx_offset);
                    }
                }
                global_idx_offset += cmd_list->IdxBuffer.Size;
                global_vtx_offset += cmd_list->VtxBuffer.Size;
            }
        });
    }
}

SKR_IMGUI_API void render_graph_imgui_add_render_pass(skr::render_graph::RenderGraph* render_graph,
    skr::render_graph::TextureRTVHandle target, ECGPULoadAction load_action)
{
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    if (!draw_data) return;

    ImGuiIO& io = ImGui::GetIO();
    // Update and Render additional Platform Windows
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        io.BackendRendererUserData = render_graph;
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0) return;
    
    imguir_render_draw_data(draw_data, render_graph, target, load_action);
}

SKR_IMGUI_API void render_graph_imgui_finalize()
{
    cgpu_free_texture(font_texture);
    cgpu_free_render_pipeline(render_pipeline);
    cgpu_free_root_signature(root_sig);
}

namespace skr::imgui
{
void imgui_create_fonts(CGPUQueueId queue)
{
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    size_t upload_size = width * height * 4 * sizeof(char);
    // create texture
    CGPUTextureDescriptor tex_desc = {};
    tex_desc.name = "imgui_font";
    tex_desc.width = static_cast<uint32_t>(width);
    tex_desc.height = static_cast<uint32_t>(height);
    tex_desc.depth = 1;
    tex_desc.descriptors = CGPU_RESOURCE_TYPE_TEXTURE;
    tex_desc.array_size = 1;
    tex_desc.flags = CGPU_TCF_OWN_MEMORY_BIT;
    tex_desc.mip_levels = 1;
    tex_desc.format = CGPU_FORMAT_R8G8B8A8_UNORM;
    tex_desc.start_state = CGPU_RESOURCE_STATE_COPY_DEST;
    tex_desc.owner_queue = queue;
    font_texture = cgpu_create_texture(queue->device, &tex_desc);
    CGPUCommandPoolDescriptor cmd_pool_desc = {};
    CGPUCommandBufferDescriptor cmd_desc = {};
    CGPUBufferDescriptor upload_buffer_desc = {};
    upload_buffer_desc.name = "IMGUI_FontUploadBuffer";
    upload_buffer_desc.flags = CGPU_BCF_OWN_MEMORY_BIT | CGPU_BCF_PERSISTENT_MAP_BIT;
    upload_buffer_desc.descriptors = CGPU_RESOURCE_TYPE_NONE;
    upload_buffer_desc.memory_usage = CGPU_MEM_USAGE_CPU_ONLY;
    upload_buffer_desc.size = upload_size;
    CGPUBufferId tex_upload_buffer = cgpu_create_buffer(queue->device, &upload_buffer_desc);
    {
        memcpy(tex_upload_buffer->cpu_mapped_address, pixels, upload_size);
    }
    auto cpy_cmd_pool = cgpu_create_command_pool(queue, &cmd_pool_desc);
    auto cpy_cmd = cgpu_create_command_buffer(cpy_cmd_pool, &cmd_desc);
    cgpu_cmd_begin(cpy_cmd);
    CGPUBufferToTextureTransfer b2t = {};
    b2t.src = tex_upload_buffer;
    b2t.src_offset = 0;
    b2t.dst = font_texture;
    b2t.dst_subresource.mip_level = 0;
    b2t.dst_subresource.base_array_layer = 0;
    b2t.dst_subresource.layer_count = 1;
    cgpu_cmd_transfer_buffer_to_texture(cpy_cmd, &b2t);
    CGPUTextureBarrier srv_barrier = {};
    srv_barrier.texture = font_texture;
    srv_barrier.src_state = CGPU_RESOURCE_STATE_COPY_DEST;
    srv_barrier.dst_state = CGPU_RESOURCE_STATE_SHADER_RESOURCE;
    CGPUResourceBarrierDescriptor barrier_desc1 = {};
    barrier_desc1.texture_barriers = &srv_barrier;
    barrier_desc1.texture_barriers_count = 1;
    cgpu_cmd_resource_barrier(cpy_cmd, &barrier_desc1);
    cgpu_cmd_end(cpy_cmd);
    CGPUQueueSubmitDescriptor cpy_submit = {};
    cpy_submit.cmds = &cpy_cmd;
    cpy_submit.cmds_count = 1;
    cgpu_submit_queue(queue, &cpy_submit);
    cgpu_wait_queue_idle(queue);
    cgpu_free_command_buffer(cpy_cmd);
    cgpu_free_command_pool(cpy_cmd_pool);
    cgpu_free_buffer(tex_upload_buffer);
    io.Fonts->TexID = (ImTextureID)(intptr_t)&font_texture;
}

void imgui_create_pipeline(const RenderGraphImGuiDescriptor* desc)
{
    CGPUPipelineShaderDescriptor ppl_shaders[2];
    ppl_shaders[0] = desc->vs;
    ppl_shaders[1] = desc->ps;
    const char8_t* push_constant_name = "push_constants";
    const char8_t* sampler_name = "sampler0";
    CGPURootSignatureDescriptor rs_desc = {};
    rs_desc.shaders = ppl_shaders;
    rs_desc.shader_count = 2;
    rs_desc.push_constant_names = &push_constant_name;
    rs_desc.push_constant_count = 1;
    rs_desc.static_sampler_names = &sampler_name;
    rs_desc.static_sampler_count = 1;
    rs_desc.static_samplers = &desc->static_sampler;
    root_sig = cgpu_create_root_signature(desc->queue->device, &rs_desc);
    CGPUVertexLayout vertex_layout = {};
    vertex_layout.attribute_count = 3;
    vertex_layout.attributes[0] = { "POSITION", 1, CGPU_FORMAT_R32G32_SFLOAT, 0, 0, sizeof(float) * 2, CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[1] = { "TEXCOORD", 1, CGPU_FORMAT_R32G32_SFLOAT, 0, sizeof(float) * 2, sizeof(float) * 2, CGPU_INPUT_RATE_VERTEX };
    vertex_layout.attributes[2] = { "COLOR", 1, CGPU_FORMAT_R8G8B8A8_UNORM, 0, sizeof(float) * 4, sizeof(uint32_t), CGPU_INPUT_RATE_VERTEX };
    CGPURasterizerStateDescriptor rs_state = {};
    rs_state.cull_mode = CGPU_CULL_MODE_NONE;
    rs_state.fill_mode = CGPU_FILL_MODE_SOLID;
    rs_state.front_face = CGPU_FRONT_FACE_CW;
    rs_state.slope_scaled_depth_bias = 0.f;
    rs_state.enable_depth_clamp = false;
    rs_state.enable_scissor = true;
    rs_state.enable_multi_sample = false;
    rs_state.depth_bias = 0;
    CGPURenderPipelineDescriptor rp_desc = {};
    rp_desc.root_signature = root_sig;
    rp_desc.prim_topology = CGPU_PRIM_TOPO_TRI_LIST;
    rp_desc.vertex_layout = &vertex_layout;
    rp_desc.vertex_shader = &ppl_shaders[0];
    rp_desc.fragment_shader = &ppl_shaders[1];
    rp_desc.render_target_count = 1;
    rp_desc.rasterizer_state = &rs_state;
    rp_desc.color_formats = &desc->backbuffer_format;
    CGPUBlendStateDescriptor blend_state = {};
    blend_state.blend_modes[0] = CGPU_BLEND_MODE_ADD;
    blend_state.src_factors[0] = CGPU_BLEND_CONST_SRC_ALPHA;
    blend_state.dst_factors[0] = CGPU_BLEND_CONST_ONE_MINUS_SRC_ALPHA;
    blend_state.blend_alpha_modes[0] = CGPU_BLEND_MODE_ADD;
    blend_state.src_alpha_factors[0] = CGPU_BLEND_CONST_ONE;
    blend_state.dst_alpha_factors[0] = CGPU_BLEND_CONST_ONE_MINUS_SRC_ALPHA;
    blend_state.masks[0] = CGPU_COLOR_MASK_ALL;
    blend_state.independent_blend = false;
    rp_desc.blend_state = &blend_state;
    render_pipeline = cgpu_create_render_pipeline(desc->queue->device, &rp_desc);
}

void imguir_render_window(ImGuiViewport* viewport, void* usrdata)
{
    ImGuiIO& io = ImGui::GetIO();
    auto graph = (skr::render_graph::RenderGraph*)io.BackendRendererUserData;

    CGPUSwapChainId swapchain = nullptr;
    if (!viewport->RendererUserData)
    {
        // create & record swapchain here
        //viewport->RendererUserData = 
    }
    // if (!(viewport->Flags & ImGuiViewportFlags_NoRendererClear));
    
    CGPUAcquireNextDescriptor acquire = {};
    acquire.fence = nullptr;
    acquire.signal_semaphore = nullptr;
    auto backbuffer_index = cgpu_acquire_next_image(swapchain, &acquire);
    CGPUTextureId native_backbuffer = swapchain->back_buffers[backbuffer_index];

    auto back_buffer = graph->create_texture(
        [=](render_graph::RenderGraph& g, render_graph::TextureBuilder& builder) {
            eastl::string res_name = "imgui-window-";
            res_name.append(eastl::to_string(viewport->ID));
            builder.set_name(res_name.c_str())
                .import(native_backbuffer, CGPU_RESOURCE_STATE_UNDEFINED)
                .allow_render_target();
        });

    imguir_render_draw_data(viewport->DrawData, graph, back_buffer, CGPU_LOAD_ACTION_CLEAR);

    graph->add_present_pass(
        [=](render_graph::RenderGraph& g, render_graph::PresentPassBuilder& builder) {
            builder.set_name("present_pass")
            .swapchain(swapchain, backbuffer_index)
            .texture(back_buffer, true);
        });
}

void imguir_create_window(ImGuiViewport* viewport)
{
    SKR_LOG_INFO("imguir_create_window");

}

void imguir_destroy_window(ImGuiViewport* viewport)
{
    SKR_LOG_INFO("imguir_destroy_window");

}

void imguir_resize_window(ImGuiViewport* viewport, ImVec2 size)
{
    SKR_LOG_INFO("imguir_resize_window");

}

void imguir_swap_buffers(ImGuiViewport* viewport, void*)
{
    // noting needs to be done here... 
}
} // namespace skr::imgui