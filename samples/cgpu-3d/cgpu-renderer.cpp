#define CGLTF_IMPLEMENTATION
#include "thirdparty/cgltf.h"

#include "render-device.hpp"
#include "render-context.hpp"
#include "render-scene.hpp"
#include <EASTL/vector_map.h>

void RenderDevice::Initialize(ECGpuBackend backend)
{
    // create sdl window
    sdl_window_ = SDL_CreateWindow(gCGpuBackendNames[backend],
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        BACK_BUFFER_WIDTH, BACK_BUFFER_HEIGHT,
        SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(sdl_window_, &wmInfo);
    // create backend device
    backend_ = backend;
    // create instance
    CGpuInstanceDescriptor instance_desc = {};
    instance_desc.backend = backend;
    instance_desc.enable_debug_layer = true;
    instance_desc.enable_gpu_based_validation = true;
    instance_desc.enable_set_name = true;
    instance_ = cgpu_create_instance(&instance_desc);
    {
        uint32_t adapters_count = 0;
        cgpu_enum_adapters(instance_, CGPU_NULLPTR, &adapters_count);
        CGpuAdapterId adapters[256];
        cgpu_enum_adapters(instance_, adapters, &adapters_count);
        adapter_ = adapters[0];
    }
    CGpuQueueGroupDescriptor Gs[2];
    Gs[0].queueType = QUEUE_TYPE_GRAPHICS;
    Gs[0].queueCount = 1;
    Gs[1].queueType = QUEUE_TYPE_TRANSFER;
    Gs[1].queueCount = 1;
    if (cgpu_query_queue_count(adapter_, QUEUE_TYPE_TRANSFER) && false)
    {
        CGpuDeviceDescriptor device_desc = {};
        device_desc.queueGroups = Gs;
        device_desc.queueGroupCount = 2;
        device_ = cgpu_create_device(adapter_, &device_desc);
        gfx_queue_ = cgpu_get_queue(device_, QUEUE_TYPE_GRAPHICS, 0);
        cpy_queue_ = cgpu_get_queue(device_, QUEUE_TYPE_TRANSFER, 0);
    }
    else
    {
        CGpuDeviceDescriptor device_desc = {};
        device_desc.queueGroups = Gs;
        device_desc.queueGroupCount = 1;
        device_ = cgpu_create_device(adapter_, &device_desc);
        gfx_queue_ = cgpu_get_queue(device_, QUEUE_TYPE_GRAPHICS, 0);
        cpy_queue_ = gfx_queue_;
    }
    // copy cmd pool
    CGpuCommandPoolDescriptor pool_desc = {};
    cpy_cmd_pool_ = cgpu_create_command_pool(cpy_queue_, &pool_desc);
#if defined(_WIN32) || defined(_WIN64)
    surface_ = cgpu_surface_from_hwnd(device_, wmInfo.info.win.window);
#elif defined(__APPLE__)
    struct CGpuNSView* ns_view = (struct CGpuNSView*)nswindow_get_content_view(wmInfo.info.cocoa.window);
    surface_ = cgpu_surface_from_ns_view(device_, ns_view);
#endif
    CGpuSwapChainDescriptor swapchain_desc = {};
    swapchain_desc.presentQueues = &gfx_queue_;
    swapchain_desc.presentQueuesCount = 1;
    swapchain_desc.width = BACK_BUFFER_WIDTH;
    swapchain_desc.height = BACK_BUFFER_HEIGHT;
    swapchain_desc.surface = surface_;
    swapchain_desc.imageCount = 3;
    swapchain_desc.format = PF_R8G8B8A8_UNORM;
    swapchain_desc.enableVsync = true;
    swapchain_ = cgpu_create_swapchain(device_, &swapchain_desc);
    for (uint32_t i = 0; i < swapchain_->buffer_count; i++)
    {
        CGpuTextureViewDescriptor view_desc = {};
        view_desc.texture = swapchain_->back_buffers[i];
        view_desc.aspects = TVA_COLOR;
        view_desc.dims = TEX_DIMENSION_2D;
        view_desc.format = (ECGpuFormat)swapchain_->back_buffers[i]->format;
        view_desc.usages = TVU_RTV_DSV;
        views_[i] = cgpu_create_texture_view(device_, &view_desc);
        // create ds
        CGpuTextureDescriptor ds_desc = {};
        eastl::string name = "DepthStencil";
        name += eastl::to_string(i);
        ds_desc.name = name.c_str();
        ds_desc.descriptors = RT_TEXTURE;
        ds_desc.flags = TCF_OWN_MEMORY_BIT;
        ds_desc.width = swapchain_->back_buffers[i]->width;
        ds_desc.height = swapchain_->back_buffers[i]->height;
        ds_desc.depth = 1;
        ds_desc.format = PF_D24_UNORM_S8_UINT;
        ds_desc.array_size = 1;
        ds_desc.start_state = RESOURCE_STATE_DEPTH_WRITE;
        ds_desc.owner_queue = gfx_queue_;
        screen_ds_[i] = cgpu_create_texture(device_, &ds_desc);
        CGpuTextureViewDescriptor ds_view_desc = {};
        ds_view_desc.texture = screen_ds_[i];
        ds_view_desc.format = PF_D24_UNORM_S8_UINT;
        ds_view_desc.array_layer_count = 1;
        ds_view_desc.base_array_layer = 0;
        ds_view_desc.mip_level_count = 1;
        ds_view_desc.base_mip_level = 0;
        ds_view_desc.aspects = TVA_DEPTH;
        ds_view_desc.dims = TEX_DIMENSION_2D;
        ds_view_desc.usages = TVU_RTV_DSV;
        screen_ds_view_[i] = cgpu_create_texture_view(device_, &ds_view_desc);
    }
    // create default shaders & resources
    CGpuShaderLibraryDescriptor vs_desc = {};
    vs_desc.name = "VertexShaderLibrary";
    vs_desc.stage = SHADER_STAGE_VERT;
    vs_desc.code = get_vertex_shader();
    vs_desc.code_size = get_vertex_shader_size();
    vs_library_ = cgpu_create_shader_library(device_, &vs_desc);
    CGpuShaderLibraryDescriptor fs_desc = {};
    fs_desc.name = "FragmentShaderLibrary";
    fs_desc.stage = SHADER_STAGE_FRAG;
    fs_desc.code = get_fragment_shader();
    fs_desc.code_size = get_fragment_shader_size();
    fs_library_ = cgpu_create_shader_library(device_, &fs_desc);
    CGpuSamplerDescriptor sampler_desc = {};
    sampler_desc.address_u = ADDRESS_MODE_REPEAT;
    sampler_desc.address_v = ADDRESS_MODE_REPEAT;
    sampler_desc.address_w = ADDRESS_MODE_REPEAT;
    sampler_desc.mipmap_mode = MIPMAP_MODE_LINEAR;
    sampler_desc.min_filter = FILTER_TYPE_LINEAR;
    sampler_desc.mag_filter = FILTER_TYPE_LINEAR;
    sampler_desc.compare_func = CMP_NEVER;
    default_sampler_ = cgpu_create_sampler(device_, &sampler_desc);
    // create root signature
    ppl_shaders_[0].stage = SHADER_STAGE_VERT;
    ppl_shaders_[0].entry = "main";
    ppl_shaders_[0].library = vs_library_;
    ppl_shaders_[1].stage = SHADER_STAGE_FRAG;
    ppl_shaders_[1].entry = "main";
    ppl_shaders_[1].library = fs_library_;
    const char8_t* sampler_name = "texture_sampler";
    const char8_t* root_constant_name = "root_constants";
    CGpuRootSignatureDescriptor rs_desc = {};
    rs_desc.shaders = ppl_shaders_;
    rs_desc.shader_count = 2;
    rs_desc.root_constant_names = &root_constant_name;
    rs_desc.root_constant_count = 1;
    rs_desc.static_samplers = &default_sampler_;
    rs_desc.static_sampler_count = 1;
    rs_desc.static_sampler_names = &sampler_name;
    root_sig_ = cgpu_create_root_signature(device_, &rs_desc);
    CGpuTextureDescriptor tex_desc = {};
    tex_desc.name = "DefaultTexture";
    tex_desc.descriptors = RT_TEXTURE;
    tex_desc.flags = TCF_OWN_MEMORY_BIT;
    tex_desc.width = TEXTURE_WIDTH;
    tex_desc.height = TEXTURE_HEIGHT;
    tex_desc.depth = 1;
    tex_desc.format = PF_R8G8B8A8_UNORM;
    tex_desc.array_size = 1;
    tex_desc.owner_queue = cpy_queue_;
    tex_desc.start_state = RESOURCE_STATE_COPY_DEST;
    default_texture_ = cgpu_create_texture(device_, &tex_desc);
    {
        // upload texture data
        CGpuBufferDescriptor upload_buffer_desc = {};
        upload_buffer_desc.name = "UploadBuffer",
        upload_buffer_desc.flags = BCF_OWN_MEMORY_BIT | BCF_PERSISTENT_MAP_BIT,
        upload_buffer_desc.descriptors = RT_NONE,
        upload_buffer_desc.memory_usage = MEM_USAGE_CPU_ONLY,
        upload_buffer_desc.element_stride = sizeof(TEXTURE_DATA),
        upload_buffer_desc.elemet_count = 1,
        upload_buffer_desc.size = sizeof(TEXTURE_DATA);
        CGpuBufferId upload_buffer = cgpu_create_buffer(device_, &upload_buffer_desc);
        // upload texture
        {
            memcpy(upload_buffer->cpu_mapped_address, TEXTURE_DATA, upload_buffer_desc.size);
        }
        CGpuCommandBufferDescriptor cmd_desc = {};
        cmd_desc.is_secondary = false;
        CGpuCommandBufferId cmd = cgpu_create_command_buffer(cpy_cmd_pool_, &cmd_desc);
        cgpu_reset_command_pool(cpy_cmd_pool_);
        cgpu_cmd_begin(cmd);
        CGpuBufferToTextureTransfer b2t = {};
        b2t.src = upload_buffer;
        b2t.src_offset = 0;
        b2t.dst = default_texture_;
        b2t.elems_per_row = TEXTURE_WIDTH;
        b2t.rows_per_image = TEXTURE_HEIGHT;
        b2t.base_array_layer = 0;
        b2t.layer_count = 1;
        cgpu_cmd_transfer_buffer_to_texture(cmd, &b2t);
        CGpuTextureBarrier srv_barrier = {};
        srv_barrier.texture = default_texture_;
        srv_barrier.src_state = RESOURCE_STATE_COPY_DEST;
        srv_barrier.dst_state = RESOURCE_STATE_SHADER_RESOURCE;
        if (cpy_queue_ != gfx_queue_)
        {
            srv_barrier.queue_release = true;
            srv_barrier.queue_type = QUEUE_TYPE_GRAPHICS;
        }
        CGpuResourceBarrierDescriptor barrier_desc = {};
        barrier_desc.texture_barriers = &srv_barrier;
        barrier_desc.texture_barriers_count = 1;
        cgpu_cmd_resource_barrier(cmd, &barrier_desc);
        cgpu_cmd_end(cmd);
        CGpuQueueSubmitDescriptor cpy_submit = {};
        cpy_submit.cmds = &cmd;
        cpy_submit.cmds_count = 1;
        cgpu_submit_queue(cpy_queue_, &cpy_submit);
        cgpu_wait_queue_idle(cpy_queue_);
        cgpu_free_buffer(upload_buffer);
        cgpu_free_command_buffer(cmd);
    }
    CGpuTextureViewDescriptor sview_desc = {};
    sview_desc.texture = default_texture_;
    sview_desc.format = tex_desc.format;
    sview_desc.array_layer_count = 1;
    sview_desc.base_array_layer = 0;
    sview_desc.mip_level_count = 1;
    sview_desc.base_mip_level = 0;
    sview_desc.aspects = TVA_COLOR;
    sview_desc.dims = TEX_DIMENSION_2D;
    sview_desc.usages = TVU_SRV;
    default_texture_view_ = cgpu_create_texture_view(device_, &sview_desc);
}

void RenderDevice::Destroy()
{
    this->WaitIdle();
    this->CollectGarbage(true);
    for (auto iter : pipelines_)
        cgpu_free_render_pipeline(iter.second);
    cgpu_free_sampler(default_sampler_);
    cgpu_free_root_signature(root_sig_);
    cgpu_free_shader_library(vs_library_);
    cgpu_free_shader_library(fs_library_);
    cgpu_free_texture_view(default_texture_view_);
    cgpu_free_texture(default_texture_);
    for (uint32_t i = 0; i < 3; i++)
    {
        if (views_[i] != nullptr) cgpu_free_texture_view(views_[i]);
        if (screen_ds_[i] != nullptr) cgpu_free_texture(screen_ds_[i]);
        if (screen_ds_view_[i] != nullptr) cgpu_free_texture_view(screen_ds_view_[i]);
    }
    cgpu_free_swapchain(swapchain_);
    cgpu_free_surface(device_, surface_);
    cgpu_free_command_pool(cpy_cmd_pool_);
    cgpu_free_queue(gfx_queue_);
    if (cpy_queue_ != gfx_queue_)
        cgpu_free_queue(cpy_queue_);
    cgpu_free_device(device_);
    cgpu_free_instance(instance_);
    SDL_DestroyWindow(sdl_window_);
}

void RenderDevice::FreeSemaphore(CGpuSemaphoreId semaphore)
{
    cgpu_free_semaphore(semaphore);
}

CGpuSemaphoreId RenderDevice::AllocSemaphore()
{
    return cgpu_create_semaphore(device_);
}

CGpuFenceId RenderDevice::AllocFence()
{
    return cgpu_create_fence(device_);
}

void RenderDevice::FreeFence(CGpuFenceId fence)
{
    cgpu_wait_fences(&fence, 1);
    cgpu_free_fence(fence);
}

void RenderDevice::asyncTransfer(const CGpuBufferToBufferTransfer* transfers,
    const ECGpuResourceState* dst_states, uint32_t transfer_count,
    CGpuSemaphoreId semaphore, CGpuFenceId fence)
{
    eastl::vector<CGpuBufferBarrier> barriers(transfer_count);
    for (uint32_t i = 0; i < barriers.size(); i++)
    {
        barriers[i].buffer = transfers[i].dst;
        barriers[i].src_state = RESOURCE_STATE_COPY_DEST;
        barriers[i].dst_state = dst_states[i];
        if (gfx_queue_ != cpy_queue_)
        {
            barriers[i].queue_release = true;
            barriers[i].queue_type = QUEUE_TYPE_GRAPHICS;
        }
    }
    CGpuCommandBufferDescriptor cmd_desc = {};
    CGpuCommandBufferId cmd = cgpu_create_command_buffer(cpy_cmd_pool_, &cmd_desc);
    cpy_cmds[semaphore] = cmd;
    cgpu_cmd_begin(cmd);
    for (uint32_t i = 0; i < transfer_count; i++)
    {
        cgpu_cmd_transfer_buffer_to_buffer(cmd, &transfers[i]);
    }
    CGpuResourceBarrierDescriptor barriers_desc = {};
    barriers_desc.buffer_barriers = barriers.data();
    barriers_desc.buffer_barriers_count = (uint32_t)barriers.size();
    cgpu_cmd_resource_barrier(cmd, &barriers_desc);
    cgpu_cmd_end(cmd);
    CGpuQueueSubmitDescriptor submit_desc = {};
    submit_desc.cmds = &cmd;
    submit_desc.cmds_count = 1;
    submit_desc.signal_semaphore_count = semaphore ? 1 : 0;
    submit_desc.signal_semaphores = semaphore ? &semaphore : nullptr;
    submit_desc.signal_fence = fence;
    cgpu_submit_queue(cpy_queue_, &submit_desc);
    async_cpy_cmds_[fence] = cmd;
}

void RenderDevice::CollectGarbage(bool wait_idle)
{
    eastl::vector<CGpuFenceId> removed;
    for (auto&& iter : async_cpy_cmds_)
    {
        if (!wait_idle)
        {
            if (cgpu_query_fence_status(iter.first) == FENCE_STATUS_COMPLETE)
            {
                cgpu_free_command_buffer(iter.second);
                removed.emplace_back(iter.first);
            }
        }
        else
        {
            cgpu_wait_fences(&iter.first, 1);
            cgpu_free_command_buffer(iter.second);
            removed.emplace_back(iter.first);
        }
    }
    for (auto&& iter : removed)
    {
        async_cpy_cmds_.erase(iter);
    }
}

CGpuRenderPipelineId RenderDevice::CreateRenderPipeline(const PipelineKey& key)
{
    auto iter = pipelines_.find(key);
    if (iter != pipelines_.end()) return iter->second;
    CGpuRasterizerStateDescriptor rasterizer_state = {};
    rasterizer_state.cull_mode = CULL_MODE_BACK;
    rasterizer_state.fill_mode = key.wireframe_mode_ ? FILL_MODE_WIREFRAME : FILL_MODE_SOLID;
    // From glTF 2.0 specification:
    // "For triangle primitives, the front face has a counter-clockwise (CCW) winding order."
    rasterizer_state.front_face = FRONT_FACE_CCW;
    rasterizer_state.slope_scaled_depth_bias = 0.f;
    rasterizer_state.enable_depth_clamp = false;
    rasterizer_state.enable_scissor = false;
    rasterizer_state.enable_multi_sample = false;
    rasterizer_state.depth_bias = 0;
    CGpuRenderPipelineDescriptor rp_desc = {};
    rp_desc.root_signature = key.root_sig_;
    rp_desc.prim_topology = PRIM_TOPO_TRI_LIST;
    auto viter = vertex_layouts_.find_by_hash(key.vertex_layout_id_);
    rp_desc.vertex_layout = &viter.get_node()->mValue;
    rp_desc.vertex_shader = &ppl_shaders_[0];
    rp_desc.fragment_shader = &ppl_shaders_[1];
    rp_desc.render_target_count = 1;
    rp_desc.rasterizer_state = &rasterizer_state;
    CGpuDepthStateDescriptor ds_state = {};
    ds_state.depth_write = true;
    ds_state.depth_test = true;
    ds_state.depth_func = CMP_GEQUAL;
    ds_state.stencil_test = false;
    rp_desc.depth_stencil_format = PF_D24_UNORM_S8_UINT;
    rp_desc.depth_state = &ds_state;
    ECGpuFormat color_format = (ECGpuFormat)swapchain_->back_buffers[0]->format;
    rp_desc.color_formats = &color_format;
    auto new_pipeline = cgpu_create_render_pipeline(device_, &rp_desc);
    pipelines_.insert({ key, new_pipeline });
    return new_pipeline;
}

CGpuDescriptorSetId RenderDevice::CreateDescriptorSet(const CGpuRootSignatureId signature, uint32_t set_index)
{
    CGpuDescriptorSetDescriptor desc_desc = {};
    desc_desc.root_signature = signature;
    desc_desc.set_index = set_index;
    auto desc_set = cgpu_create_descriptor_set(device_, &desc_desc);
    CGpuDescriptorData arguments[1];
    arguments[0].name = "sampled_texture";
    arguments[0].count = 1;
    arguments[0].textures = &default_texture_view_;
    cgpu_update_descriptor_set(desc_set, arguments, 1);
    return desc_set;
}

void RenderDevice::FreeDescriptorSet(CGpuDescriptorSetId desc_set)
{
    cgpu_free_descriptor_set(desc_set);
}

uint32_t RenderDevice::AcquireNextFrame(const CGpuAcquireNextDescriptor& acquire)
{
    return cgpu_acquire_next_image(swapchain_, &acquire);
}

void RenderDevice::Present(uint32_t index, const CGpuSemaphoreId* wait_semaphores, uint32_t semaphore_count)
{
    CGpuQueuePresentDescriptor present_desc = {};
    present_desc.index = index;
    present_desc.wait_semaphore_count = semaphore_count;
    present_desc.wait_semaphores = wait_semaphores;
    present_desc.swapchain = swapchain_;
    cgpu_queue_present(gfx_queue_, &present_desc);
}

void RenderDevice::Submit(class RenderContext* context)
{
    CGpuQueueSubmitDescriptor submit_desc = {};
    submit_desc.cmds = &context->cmd_buffer_;
    submit_desc.cmds_count = 1;
    submit_desc.signal_fence = context->exec_fence_;
    cgpu_submit_queue(gfx_queue_, &submit_desc);
}

void RenderDevice::WaitIdle()
{
    cgpu_wait_queue_idle(gfx_queue_);
    cgpu_wait_queue_idle(cpy_queue_);
}

void RenderDevice::freeRenderPipeline(CGpuRenderPipelineId pipeline)
{
    cgpu_free_render_pipeline(pipeline);
}

void RenderContext::Initialize(RenderDevice* device)
{
    device_ = device;
    CGpuCommandPoolDescriptor pool_desc = {};
    cmd_pool_ = cgpu_create_command_pool(device->GetCGPUQueue(), &pool_desc);
    CGpuCommandBufferDescriptor cmd_desc = {};
    cmd_desc.is_secondary = false;
    cmd_buffer_ = cgpu_create_command_buffer(cmd_pool_, &cmd_desc);
    exec_fence_ = cgpu_create_fence(device->GetCGPUDevice());
}
void RenderContext::Begin()
{
    cgpu_wait_fences(&exec_fence_, 1);
    cgpu_reset_command_pool(cmd_pool_);
    cgpu_cmd_begin(cmd_buffer_);
}

void RenderContext::ResourceBarrier(const CGpuResourceBarrierDescriptor& barrier_desc)
{
    cgpu_cmd_resource_barrier(cmd_buffer_, &barrier_desc);
}

void RenderContext::BeginRenderPass(const CGpuRenderPassDescriptor& rp_desc)
{
    rp_encoder_ = cgpu_cmd_begin_render_pass(cmd_buffer_, &rp_desc);
}

void RenderContext::SetViewport(float x, float y, float width, float height, float min_depth, float max_depth)
{
    cgpu_render_encoder_set_viewport(rp_encoder_, x, y, width, height, min_depth, max_depth);
}

void RenderContext::SetScissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    cgpu_render_encoder_set_scissor(rp_encoder_, x, y, width, height);
}

void RenderContext::BindPipeline(CGpuRenderPipelineId pipeline)
{
    cgpu_render_encoder_bind_pipeline(rp_encoder_, pipeline);
}

void RenderContext::BindVertexBuffers(uint32_t buffer_count, const CGpuBufferId* buffers, const uint32_t* strides, const uint32_t* offsets)
{
    cgpu_render_encoder_bind_vertex_buffers(rp_encoder_, buffer_count, buffers, strides, offsets);
}

void RenderContext::BindIndexBuffer(CGpuBufferId buffer, uint32_t index_stride, uint64_t offset)
{
    cgpu_render_encoder_bind_index_buffer(rp_encoder_, buffer, index_stride, offset);
}

void RenderContext::BindDescriptorSet(CGpuDescriptorSetId desc_set)
{
    cgpu_render_encoder_bind_descriptor_set(rp_encoder_, desc_set);
}

void RenderContext::PushConstants(CGpuRootSignatureId rs, const char8_t* name, const void* data)
{
    cgpu_render_encoder_push_constants(rp_encoder_, rs, name, data);
}

void RenderContext::DrawIndexedInstanced(uint32_t index_count, uint32_t first_index, uint32_t instance_count, uint32_t first_instance, uint32_t first_vertex)
{
    cgpu_render_encoder_draw_indexed_instanced(rp_encoder_, index_count, first_index, instance_count, first_instance, first_vertex);
}

void RenderContext::EndRenderPass()
{
    cgpu_cmd_end_render_pass(cmd_buffer_, rp_encoder_);
    rp_encoder_ = nullptr;
}

void RenderContext::End()
{
    cgpu_cmd_end(cmd_buffer_);
}

void RenderContext::Wait()
{
    if (cgpu_query_fence_status(exec_fence_) == FENCE_STATUS_INCOMPLETE)
        cgpu_wait_fences(&exec_fence_, 1);
}

void RenderContext::Destroy()
{
    auto status = cgpu_query_fence_status(exec_fence_);
    if (status == FENCE_STATUS_INCOMPLETE)
    {
        cgpu_wait_fences(&exec_fence_, 1);
    }
    cgpu_free_fence(exec_fence_);
    cgpu_reset_command_pool(cmd_pool_);
    cgpu_free_command_buffer(cmd_buffer_);
    cgpu_free_command_pool(cmd_pool_);
}

int32_t RenderMesh::loadPrimitive(struct cgltf_primitive* src, uint32_t& index_cursor)
{
    RenderPrimitive newPrim = {};
    newPrim.index_offset_ = (uint32_t)src->indices->offset;
    newPrim.index_count_ = (uint32_t)src->indices->count;
    newPrim.first_index_ = index_cursor;
    newPrim.vertex_strides_.reserve(src->attributes_count);
    newPrim.vertex_offsets_.reserve(src->attributes_count);
    newPrim.vertex_buffers_.reserve(src->attributes_count);
    for (uint32_t i = 0; i < src->attributes_count; i++)
    {
        auto attrib = src->attributes + i;
        newPrim.vertex_strides_.emplace_back((uint32_t)attrib->data->stride);
        newPrim.vertex_offsets_.emplace_back((uint32_t)attrib->data->offset);
    }
    primitives_.emplace_back(newPrim);
    index_cursor += newPrim.index_count_;
    return (int32_t)primitives_.size() - 1;
}

int32_t RenderScene::loadMesh(struct cgltf_mesh* src)
{
    RenderMesh newMesh = {};
    newMesh.name_ = src->name;
    uint32_t index_cursor = 0;
    newMesh.primitives_.reserve(src->primitives_count);
    for (uint32_t i = 0; i < src->primitives_count; i++)
    {
        auto gltf_prim = src->primitives + i;
        newMesh.loadPrimitive(gltf_prim, index_cursor);
    }
    meshes_.emplace_back(newMesh);
    return (int32_t)meshes_.size() - 1;
}

int32_t RenderScene::loadNode(struct cgltf_node* src, int32_t parent_idx)
{
    const bool isRoot = (parent_idx == -1);
    nodes_.emplace_back();
    RenderNode& newNode = nodes_[nodes_.size() - 1];
    newNode.index_ = (int32_t)nodes_.size() - 1;
    newNode.parent_ = isRoot ? nullptr : &nodes_[parent_idx];
    newNode.children_.reserve(src->children_count);
    for (uint32_t i = 0; i < src->children_count; i++)
    {
        int32_t child_idx = loadNode(src->children[i], newNode.index_);
        newNode.children_.emplace_back(&nodes_[child_idx]);
    }
    newNode.translation_ = sakura::math::Vector3f(
        src->translation[0], src->translation[1], src->translation[2]);
    newNode.rotation_ = sakura::math::Quaternion(
        src->rotation[0], src->rotation[1], src->rotation[2], src->rotation[3]);
    newNode.scale_ = sakura::math::Vector3f(
        src->scale[0], src->scale[1], src->scale[2]);
    return newNode.index_;
}

void RenderScene::Initialize(const char8_t* path)
{
    cgltf_options options = {};
    // file input
    if (path)
    {
        cgltf_result result = cgltf_parse_file(&options, path, &gltf_data_);
        if (result != cgltf_result_success)
        {
            gltf_data_ = nullptr;
            return;
        }
        else
        {
            result = cgltf_load_buffers(&options, gltf_data_, path);
            result = cgltf_validate(gltf_data_);
            if (result != cgltf_result_success)
            {
                return;
            }
        }
    }
    // construct
    {
        // load meshes
        meshes_.reserve(gltf_data_->meshes_count);
        for (uint32_t i = 0; i < gltf_data_->meshes_count; i++)
        {
            auto gltf_mesh = gltf_data_->meshes + i;
            loadMesh(gltf_mesh);
        }
        // load nodes
        nodes_.reserve(gltf_data_->nodes_count);
        for (uint32_t i = 0; i < gltf_data_->nodes_count; i++)
        {
            auto gltf_node = gltf_data_->nodes + i;
            if (gltf_node->parent == nullptr)
            {
                root_node_index_ = loadNode(gltf_node, -1);
                break;
            }
        }
    }
    load_ready_ = true;
}

static FORCEINLINE ECGpuFormat GLTFUtil_ComponentTypeToFormat(cgltf_type type, cgltf_component_type comp_type)
{
    switch (type)
    {
        case cgltf_type_scalar: {
            switch (comp_type)
            {
                case cgltf_component_type_r_8:
                    return PF_R8_SNORM;
                case cgltf_component_type_r_8u:
                    return PF_R8_UNORM;
                case cgltf_component_type_r_16:
                    return PF_R16_SINT;
                case cgltf_component_type_r_16u:
                    return PF_R16_UINT;
                case cgltf_component_type_r_32u:
                    return PF_R32_UINT;
                case cgltf_component_type_r_32f:
                    return PF_R32_SFLOAT;
                default:
                    return PF_R8_SNORM;
            }
        }
        case cgltf_type_vec2: {
            switch (comp_type)
            {
                case cgltf_component_type_r_8:
                    return PF_R8G8_SNORM;
                case cgltf_component_type_r_8u:
                    return PF_R8G8_UNORM;
                case cgltf_component_type_r_16:
                    return PF_R16G16_SINT;
                case cgltf_component_type_r_16u:
                    return PF_R16G16_UINT;
                case cgltf_component_type_r_32u:
                    return PF_R32G32_UINT;
                case cgltf_component_type_r_32f:
                    return PF_R32G32_SFLOAT;
                default:
                    return PF_R8_SNORM;
            }
        }
        case cgltf_type_vec3: {
            switch (comp_type)
            {
                case cgltf_component_type_r_8:
                    return PF_R8G8B8_SNORM;
                case cgltf_component_type_r_8u:
                    return PF_R8G8B8_UNORM;
                case cgltf_component_type_r_16:
                    return PF_R16G16B16_SINT;
                case cgltf_component_type_r_16u:
                    return PF_R16G16B16_UINT;
                case cgltf_component_type_r_32u:
                    return PF_R32G32B32_UINT;
                case cgltf_component_type_r_32f:
                    return PF_R32G32B32_SFLOAT;
                default:
                    return PF_R8_SNORM;
            }
        }
        case cgltf_type_vec4: {
            switch (comp_type)
            {
                case cgltf_component_type_r_8:
                    return PF_R8G8B8A8_SNORM;
                case cgltf_component_type_r_8u:
                    return PF_R8G8B8A8_UNORM;
                case cgltf_component_type_r_16:
                    return PF_R16G16B16A16_SINT;
                case cgltf_component_type_r_16u:
                    return PF_R16G16B16A16_UINT;
                case cgltf_component_type_r_32u:
                    return PF_R32G32B32A32_UINT;
                case cgltf_component_type_r_32f:
                    return PF_R32G32B32A32_SFLOAT;
                default:
                    return PF_R8_SNORM;
            }
        }
        default:
            return PF_R8_SNORM;
    }
    return PF_R8_SNORM;
}

static const char8_t* gGLTFAttributeTypeLUT[] = {
    "NONE",
    "POSITION",
    "NORMAL",
    "TANGENT",
    "TEXCOORD",
    "COLOR",
    "JOINTS",
    "WEIGHTS"
};

void RenderScene::Upload(RenderContext* context, bool keep_gltf_data_)
{
    context_ = context;
    auto renderDevice = context->GetRenderDevice();
    if (load_ready_)
    {
        // create buffers
        eastl::vector_map<cgltf_buffer_view*, CGpuBufferId> viewVBIdxMap = {};
        cgltf_buffer_view* indices_view = nullptr;
        eastl::vector_map<uint32_t, uint32_t> bindingOffsetMap = {};
        for (uint32_t i = 0; i < gltf_data_->buffer_views_count; i++)
        {
            cgltf_buffer_view* buf_view = gltf_data_->buffer_views + i;
            CGpuBufferDescriptor buffer_desc = {};
            buffer_desc.flags = BCF_OWN_MEMORY_BIT;
            buffer_desc.memory_usage = MEM_USAGE_GPU_ONLY;
            buffer_desc.descriptors =
                buf_view->type == cgltf_buffer_view_type_indices ?
                    RT_INDEX_BUFFER :
                    RT_VERTEX_BUFFER;
            buffer_desc.element_stride = buf_view->stride ? buf_view->stride : buf_view->size;
            buffer_desc.elemet_count = buf_view->size / buffer_desc.element_stride;
            buffer_desc.size = buf_view->size;
            buffer_desc.name = buf_view->name;
            if (buf_view->type == cgltf_buffer_view_type_indices)
            {
                indices_view = buf_view;
                index_buffer_ = cgpu_create_buffer(renderDevice->GetCGPUDevice(), &buffer_desc);
                index_stride_ = gltf_data_->accessors->component_type;
            }
            else
            {
                auto vertex_buffer_ = cgpu_create_buffer(renderDevice->GetCGPUDevice(), &buffer_desc);
                vertex_buffers_.emplace_back(vertex_buffer_);
                viewVBIdxMap[buf_view] = vertex_buffer_;
                bindingOffsetMap[(uint32_t)vertex_buffers_.size() - 1] = 0;
            }
        }
        // create vertex layout
        for (uint32_t i = 0; i < gltf_data_->meshes_count; i++)
        {
            auto gltf_mesh = gltf_data_->meshes + i;
            uint32_t location = 0;
            for (uint32_t j = 0; j < gltf_mesh->primitives_count; j++)
            {
                auto gltf_prim = gltf_mesh->primitives + j;
                CGpuVertexLayout layout = {};
                uint32_t binding = 0;
                layout.attribute_count = (uint32_t)gltf_prim->attributes_count;
                for (uint32_t k = 0; k < gltf_prim->attributes_count; k++)
                {
                    const auto gltf_attrib = gltf_prim->attributes + k;
                    const char8_t* attr_name = gGLTFAttributeTypeLUT[gltf_attrib->type];
                    strcpy(layout.attributes[k].semantic_name, attr_name);
                    layout.attributes[k].rate = INPUT_RATE_VERTEX;
                    layout.attributes[k].format = GLTFUtil_ComponentTypeToFormat(
                        gltf_attrib->data->type, gltf_attrib->data->component_type);
                    auto&& iter = viewVBIdxMap.find(gltf_attrib->data->buffer_view);
                    if (iter != viewVBIdxMap.end())
                    {
                        const auto offset = bindingOffsetMap[binding];
                        layout.attributes[k].binding = binding;
                        binding += 1;
                        layout.attributes[k].offset = 0;
                        bindingOffsetMap[binding] = (uint32_t)offset + (uint32_t)gltf_attrib->data->stride;
                        layout.attributes[k].location = location;
                        location = location + 1;
                    }
                    meshes_[i].primitives_[j].vertex_buffers_.emplace_back(
                        viewVBIdxMap[gltf_attrib->data->buffer_view]);
                }
                // pso
                PipelineKey pplKey = {};
                pplKey.vertex_layout_id_ = (uint32_t)renderDevice->AddVertexLayout(layout);
                pplKey.root_sig_ = renderDevice->GetCGPUSignature();
                pplKey.wireframe_mode_ = false;
                meshes_[i].primitives_[j].pipeline_ = renderDevice->CreateRenderPipeline(pplKey);
                meshes_[i].primitives_[j].desc_set_ = renderDevice->CreateDescriptorSet(pplKey.root_sig_, 0);
            }
        }
        gpu_memory_ready = true;
        gpu_geometry_fence = renderDevice->AllocFence();
        // staging buffer
        eastl::vector_map<const cgltf_buffer*, eastl::pair<uint32_t, uint32_t>> bufferRangeMap = {};
        size_t staging_size = 0;
        for (uint32_t i = 0; i < gltf_data_->buffers_count; i++)
        {
            const size_t range_start = staging_size;
            staging_size += gltf_data_->buffers[i].size;
            bufferRangeMap[gltf_data_->buffers + i] = { (uint32_t)range_start, (uint32_t)staging_size };
        }
        CGpuBufferDescriptor upload_buffer_desc = {};
        upload_buffer_desc.flags = BCF_OWN_MEMORY_BIT | BCF_PERSISTENT_MAP_BIT;
        upload_buffer_desc.descriptors = RT_NONE;
        upload_buffer_desc.memory_usage = MEM_USAGE_CPU_ONLY;
        upload_buffer_desc.element_stride = staging_size;
        upload_buffer_desc.elemet_count = 1;
        upload_buffer_desc.size = staging_size;
        staging_buffer_ = cgpu_create_buffer(renderDevice->GetCGPUDevice(), &upload_buffer_desc);
        // upload buffers
        {
            char8_t* address_cursor = (char8_t*)staging_buffer_->cpu_mapped_address;
            for (uint32_t i = 0; i < gltf_data_->buffers_count; i++)
            {
                memcpy(address_cursor, gltf_data_->buffers[i].data, gltf_data_->buffers[i].size);
                address_cursor += gltf_data_->buffers[i].size;
            }
        }
        eastl::vector<ECGpuResourceState> dst_states(viewVBIdxMap.size() + 1);
        eastl::vector<CGpuBufferToBufferTransfer> transfers(viewVBIdxMap.size() + 1);
        // transfer
        transfers[0].src = staging_buffer_;
        transfers[0].src_offset = bufferRangeMap[indices_view->buffer].first + indices_view->offset;
        transfers[0].dst = index_buffer_;
        transfers[0].dst_offset = 0;
        transfers[0].size = indices_view->size;
        dst_states[0] = RESOURCE_STATE_INDEX_BUFFER;
        for (uint32_t i = 1; i < viewVBIdxMap.size() + 1; i++)
        {
            const cgltf_buffer_view* cgltfBufferView = viewVBIdxMap.at(i - 1).first;
            const cgltf_buffer* cgltfBuffer = cgltfBufferView->buffer;
            transfers[i].src = staging_buffer_;
            transfers[i].src_offset = bufferRangeMap[cgltfBuffer].first + cgltfBufferView->offset;
            transfers[i].dst = vertex_buffers_[i - 1];
            transfers[i].dst_offset = 0;
            transfers[i].size = cgltfBufferView->size;
            dst_states[i] = RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
        }
        gpu_geometry_semaphore = renderDevice->AsyncTransfer(
            transfers.data(), dst_states.data(), (uint32_t)transfers.size(), gpu_geometry_fence);
    }
}

void RenderScene::Destroy()
{
    auto renderDevice = context_->GetRenderDevice();
    if (gltf_data_) cgltf_free(gltf_data_);
    if (gpu_geometry_fence)
    {
        cgpu_wait_fences(&gpu_geometry_fence, 1);
        renderDevice->FreeFence(gpu_geometry_fence);
        renderDevice->FreeSemaphore(gpu_geometry_semaphore);
        // TODO: Refactor this
        renderDevice->WaitIdle();
        for (auto& mesh : meshes_)
        {
            for (auto& prim : mesh.primitives_)
            {
                renderDevice->FreeDescriptorSet(prim.desc_set_);
            }
        }
        if (staging_buffer_) cgpu_free_buffer(staging_buffer_);
    }
    if (index_buffer_) cgpu_free_buffer(index_buffer_);
    for (auto vertex_buffer_ : vertex_buffers_)
    {
        if (vertex_buffer_) cgpu_free_buffer(vertex_buffer_);
    }
}