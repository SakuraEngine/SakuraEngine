#include "platform/configure.h"
#include "imgui/imgui.h"
#include "imgui/skr_imgui.h"

namespace sakura::imgui
{
RUNTIME_API CGpuTextureId font_texture;
RUNTIME_API CGpuRootSignatureId root_sig;
RUNTIME_API CGpuRenderPipelineId render_pipeline;
RUNTIME_API sakura::render_graph::TextureHandle font_handle;
RUNTIME_API sakura::render_graph::BufferHandle vertex_buffer_handle;
RUNTIME_API sakura::render_graph::BufferHandle index_buffer_handle;
RUNTIME_API sakura::render_graph::BufferHandle upload_buffer_handle;
RUNTIME_API CGpuBufferId upload_buffer;

RUNTIME_API ImGuiContext*& imgui_context()
{
    static thread_local ImGuiContext* ctx = nullptr;
    return ctx;
}

namespace rg = sakura::render_graph;

void imgui_create_fonts(CGpuQueueId queue);
void imgui_create_pipeline(const RenderGraphImGuiDescriptor* desc);
void imgui_render_window(ImGuiViewport* viewport, void*);
} // namespace sakura::imgui

using namespace sakura::imgui;
RUNTIME_API void render_graph_imgui_initialize(const RenderGraphImGuiDescriptor* desc)
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
    platform_io.Renderer_RenderWindow = imgui_render_window;
}

RUNTIME_API void render_graph_imgui_add_render_pass(
    sakura::render_graph::RenderGraph* render_graph,
    sakura::render_graph::TextureRTVHandle target,
    ECGpuLoadAction load_action)
{
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();
    if (!draw_data)
        return;

    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    int fb_width = (int)(draw_data->DisplaySize.x * draw_data->FramebufferScale.x);
    int fb_height = (int)(draw_data->DisplaySize.y * draw_data->FramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0)
        return;
    if (draw_data->TotalVtxCount > 0)
    {
        // create or resize the vertex/index buffers
        auto device = render_graph->get_backend_device();
        size_t vertex_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
        size_t index_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);
        font_handle = render_graph->create_texture(
            [=](rg::RenderGraph& g, rg::TextureBuilder& builder) {
                builder.set_name("imgui_font_texture")
                    .import(font_texture, RESOURCE_STATE_SHADER_RESOURCE);
            });
        if ((!upload_buffer || upload_buffer->size < index_size + vertex_size) && device)
        {
            if (upload_buffer)
            {
                cgpu_wait_queue_idle(render_graph->get_gfx_queue());
                cgpu_free_buffer(upload_buffer);
            }
            upload_buffer = cgpux_create_mapped_upload_buffer(
                device, index_size + vertex_size,
                "imgui_upload_buffer");
        }
        // upload
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
        vertex_buffer_handle = render_graph->create_buffer(
            [=](rg::RenderGraph& g, rg::BufferBuilder& builder) {
                builder.set_name("imgui_vertex_buffer")
                    .size(vertex_size)
                    .memory_usage(MEM_USAGE_GPU_ONLY)
                    .as_vertex_buffer();
            });
        index_buffer_handle = render_graph->create_buffer(
            [=](rg::RenderGraph& g, rg::BufferBuilder& builder) {
                builder.set_name("imgui_index_buffer")
                    .size(index_size)
                    .memory_usage(MEM_USAGE_GPU_ONLY)
                    .as_index_buffer();
            });
        upload_buffer_handle = render_graph->create_buffer(
            [=](rg::RenderGraph& g, rg::BufferBuilder& builder) {
                builder.set_name("imgui_upload_buffer")
                    .import(upload_buffer, RESOURCE_STATE_COPY_SOURCE);
            });
        render_graph->add_copy_pass(
            [=](rg::RenderGraph& g, rg::CopyPassBuilder& builder) {
                builder.set_name("imgui_geom_transfer")
                    .buffer_to_buffer(upload_buffer_handle.range(0, vertex_size), vertex_buffer_handle.range(0, vertex_size))
                    .buffer_to_buffer(upload_buffer_handle.range(vertex_size, vertex_size + index_size), index_buffer_handle.range(0, index_size));
            });
        // add pass
        render_graph->add_render_pass(
            [=](rg::RenderGraph& g, rg::RenderPassBuilder& builder) {
                builder.set_name("imgui_pass")
                    .set_pipeline(render_pipeline)
                    .use_buffer(vertex_buffer_handle, RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER)
                    .use_buffer(index_buffer_handle, RESOURCE_STATE_INDEX_BUFFER)
                    .read("texture0", font_handle)
                    .write(0, target, load_action);
            },
            [target](rg::RenderGraph& g, rg::RenderPassContext& context) {
                auto target_node = g.resolve(target);
                const auto& target_desc = target_node->get_desc();
                struct {
                    float inv_x;
                    float inv_y;
                } invDisplaySize;
                invDisplaySize.inv_x = 1.f / (float)target_desc.width;
                invDisplaySize.inv_y = 1.f / (float)target_desc.height;
                cgpu_render_encoder_set_viewport(context.encoder,
                    0.0f, 0.0f,
                    (float)target_desc.width,
                    (float)target_desc.height,
                    0.f, 1.f);
                cgpu_render_encoder_push_constants(context.encoder, root_sig, "root_constants", &invDisplaySize);
                // drawcalls
                ImDrawData* draw_data = ImGui::GetDrawData();
                // Will project scissor/clipping rectangles into framebuffer space
                ImVec2 clip_off = draw_data->DisplayPos;         // (0,0) unless using multi-viewports
                ImVec2 clip_scale = draw_data->FramebufferScale; // (1,1) unless using retina display which are often (2,2)
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
                            cgpu_render_encoder_bind_index_buffer(context.encoder,
                                context.resolve(index_buffer_handle), sizeof(uint16_t), 0);
                            const uint32_t stride = sizeof(ImDrawVert);
                            auto resolved_vb = context.resolve(vertex_buffer_handle);
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

RUNTIME_API void render_graph_imgui_finalize()
{
    if (upload_buffer) cgpu_free_buffer(upload_buffer);
    cgpu_free_texture(font_texture);
    cgpu_free_render_pipeline(render_pipeline);
    cgpu_free_root_signature(root_sig);
}

namespace sakura::imgui
{
void imgui_create_fonts(CGpuQueueId queue)
{
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    size_t upload_size = width * height * 4 * sizeof(char);
    // create texture
    CGpuTextureDescriptor tex_desc = {};
    tex_desc.name = "imgui_font";
    tex_desc.width = static_cast<uint32_t>(width);
    tex_desc.height = static_cast<uint32_t>(height);
    tex_desc.depth = 1;
    tex_desc.descriptors = RT_TEXTURE;
    tex_desc.array_size = 1;
    tex_desc.flags = TCF_OWN_MEMORY_BIT;
    tex_desc.mip_levels = 1;
    tex_desc.format = PF_R8G8B8A8_UNORM;
    tex_desc.start_state = RESOURCE_STATE_COPY_DEST;
    tex_desc.owner_queue = queue;
    font_texture = cgpu_create_texture(queue->device, &tex_desc);
    CGpuCommandPoolDescriptor cmd_pool_desc = {};
    CGpuCommandBufferDescriptor cmd_desc = {};
    CGpuBufferDescriptor upload_buffer_desc = {};
    upload_buffer_desc.name = "IMGUI_FontUploadBuffer";
    upload_buffer_desc.flags = BCF_OWN_MEMORY_BIT | BCF_PERSISTENT_MAP_BIT;
    upload_buffer_desc.descriptors = RT_NONE;
    upload_buffer_desc.memory_usage = MEM_USAGE_CPU_ONLY;
    upload_buffer_desc.size = upload_size;
    CGpuBufferId tex_upload_buffer = cgpu_create_buffer(queue->device, &upload_buffer_desc);
    {
        memcpy(tex_upload_buffer->cpu_mapped_address, pixels, upload_size);
    }
    auto cpy_cmd_pool = cgpu_create_command_pool(queue, &cmd_pool_desc);
    auto cpy_cmd = cgpu_create_command_buffer(cpy_cmd_pool, &cmd_desc);
    cgpu_cmd_begin(cpy_cmd);
    CGpuBufferToTextureTransfer b2t = {};
    b2t.src = tex_upload_buffer;
    b2t.src_offset = 0;
    b2t.dst = font_texture;
    b2t.dst_subresource.mip_level = 0;
    b2t.dst_subresource.base_array_layer = 0;
    b2t.dst_subresource.layer_count = 1;
    cgpu_cmd_transfer_buffer_to_texture(cpy_cmd, &b2t);
    CGpuTextureBarrier srv_barrier = {};
    srv_barrier.texture = font_texture;
    srv_barrier.src_state = RESOURCE_STATE_COPY_DEST;
    srv_barrier.dst_state = RESOURCE_STATE_SHADER_RESOURCE;
    CGpuResourceBarrierDescriptor barrier_desc1 = {};
    barrier_desc1.texture_barriers = &srv_barrier;
    barrier_desc1.texture_barriers_count = 1;
    cgpu_cmd_resource_barrier(cpy_cmd, &barrier_desc1);
    cgpu_cmd_end(cpy_cmd);
    CGpuQueueSubmitDescriptor cpy_submit = {};
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
    CGpuPipelineShaderDescriptor ppl_shaders[2];
    ppl_shaders[0] = desc->vs;
    ppl_shaders[1] = desc->ps;
    const char8_t* root_constant_name = "root_constants";
    const char8_t* sampler_name = "sampler0";
    CGpuRootSignatureDescriptor rs_desc = {};
    rs_desc.shaders = ppl_shaders;
    rs_desc.shader_count = 2;
    rs_desc.root_constant_names = &root_constant_name;
    rs_desc.root_constant_count = 1;
    rs_desc.static_sampler_names = &sampler_name;
    rs_desc.static_sampler_count = 1;
    rs_desc.static_samplers = &desc->static_sampler;
    root_sig = cgpu_create_root_signature(desc->queue->device, &rs_desc);
    CGpuVertexLayout vertex_layout = {};
    vertex_layout.attribute_count = 3;
    vertex_layout.attributes[0] = { "POSITION", 1, PF_R32G32_SFLOAT, 0, 0, sizeof(float) * 2, INPUT_RATE_VERTEX };
    vertex_layout.attributes[1] = { "TEXCOORD", 1, PF_R32G32_SFLOAT, 0, sizeof(float) * 2, sizeof(float) * 2, INPUT_RATE_VERTEX };
    vertex_layout.attributes[2] = { "COLOR", 1, PF_R8G8B8A8_UNORM, 0, sizeof(float) * 4, sizeof(uint32_t), INPUT_RATE_VERTEX };
    CGpuRasterizerStateDescriptor rs_state = {};
    rs_state.cull_mode = CULL_MODE_NONE;
    rs_state.fill_mode = FILL_MODE_SOLID;
    rs_state.front_face = FRONT_FACE_CW;
    rs_state.slope_scaled_depth_bias = 0.f;
    rs_state.enable_depth_clamp = false;
    rs_state.enable_scissor = true;
    rs_state.enable_multi_sample = false;
    rs_state.depth_bias = 0;
    CGpuRenderPipelineDescriptor rp_desc = {};
    rp_desc.root_signature = root_sig;
    rp_desc.prim_topology = PRIM_TOPO_TRI_LIST;
    rp_desc.vertex_layout = &vertex_layout;
    rp_desc.vertex_shader = &ppl_shaders[0];
    rp_desc.fragment_shader = &ppl_shaders[1];
    rp_desc.render_target_count = 1;
    rp_desc.rasterizer_state = &rs_state;
    rp_desc.color_formats = &desc->backbuffer_format;
    CGpuBlendStateDescriptor blend_state = {};
    blend_state.blend_modes[0] = BLEND_MODE_ADD;
    blend_state.src_factors[0] = BLEND_CONST_SRC_ALPHA;
    blend_state.dst_factors[0] = BLEND_CONST_ONE_MINUS_SRC_ALPHA;
    blend_state.blend_alpha_modes[0] = BLEND_MODE_ADD;
    blend_state.src_alpha_factors[0] = BLEND_CONST_ONE;
    blend_state.dst_alpha_factors[0] = BLEND_CONST_ONE_MINUS_SRC_ALPHA;
    blend_state.masks[0] = COLOR_MASK_ALL;
    blend_state.independent_blend = false;
    rp_desc.blend_state = &blend_state;
    render_pipeline = cgpu_create_render_pipeline(desc->queue->device, &rp_desc);
}

void imgui_render_window(ImGuiViewport* viewport, void*)
{
    if (!(viewport->Flags & ImGuiViewportFlags_NoRendererClear))
    {
    }
}

} // namespace sakura::imgui