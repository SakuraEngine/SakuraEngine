#include "cgpu/flags.h"
#include "render-device.hpp"
#include "render-context.hpp"
#include "render-scene.hpp"
#include "render-resources.hpp"
#include <EASTL/vector_map.h>

#define MAX_CPY_QUEUE_COUNT 1

void RenderWindow::Initialize(RenderDevice* render_device)
{
    render_device_ = render_device;
    auto device_ = render_device->GetCGPUDevice();
    // create sdl window
    sdl_window_ = SDL_CreateWindow(gCGPUBackendNames[render_device->backend_],
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    BACK_BUFFER_WIDTH, BACK_BUFFER_HEIGHT,
    SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(sdl_window_, &wmInfo);
#if defined(_WIN32) || defined(_WIN64)
    surface_ = cgpu_surface_from_hwnd(device_, wmInfo.info.win.window);
#elif defined(__APPLE__)
    struct CGPUNSView* ns_view = (struct CGPUNSView*)nswindow_get_content_view(wmInfo.info.cocoa.window);
    surface_ = cgpu_surface_from_ns_view(device_, ns_view);
#endif
    CGPUSwapChainDescriptor swapchain_desc = {};
    swapchain_desc.presentQueues = &render_device->gfx_queue_;
    swapchain_desc.presentQueuesCount = 1;
    swapchain_desc.width = BACK_BUFFER_WIDTH;
    swapchain_desc.height = BACK_BUFFER_HEIGHT;
    swapchain_desc.surface = surface_;
    swapchain_desc.imageCount = 3;
    swapchain_desc.format = CGPU_FORMAT_R8G8B8A8_UNORM;
    swapchain_desc.enableVsync = false;
    swapchain_ = cgpu_create_swapchain(device_, &swapchain_desc);
    for (uint32_t i = 0; i < swapchain_->buffer_count; i++)
    {
        CGPUTextureViewDescriptor view_desc = {};
        view_desc.texture = swapchain_->back_buffers[i];
        view_desc.aspects = CGPU_TVA_COLOR;
        view_desc.dims = CGPU_TEX_DIMENSION_2D;
        view_desc.format = (ECGPUFormat)swapchain_->back_buffers[i]->format;
        view_desc.usages = CGPU_TVU_RTV_DSV;
        views_[i] = cgpu_create_texture_view(device_, &view_desc);
    }
    // create msaa resolve views
    if (RenderWindow::SampleCount != CGPU_SAMPLE_COUNT_1)
    {
        CGPUTextureDescriptor resolve_desc = {};
        eastl::string name2 = "MSAAResolve";
        resolve_desc.name = name2.c_str();
        resolve_desc.descriptors = CGPU_RESOURCE_TYPE_TEXTURE | CGPU_RESOURCE_TYPE_RENDER_TARGET;
        resolve_desc.flags = CGPU_TCF_OWN_MEMORY_BIT;
        resolve_desc.width = swapchain_->back_buffers[0]->width;
        resolve_desc.height = swapchain_->back_buffers[0]->height;
        resolve_desc.depth = 1;
        resolve_desc.format = (ECGPUFormat)swapchain_->back_buffers[0]->format;
        resolve_desc.array_size = 1;
        resolve_desc.sample_count = RenderWindow::SampleCount;
        resolve_desc.start_state = CGPU_RESOURCE_STATE_RENDER_TARGET;
        resolve_desc.owner_queue = render_device->GetGraphicsQueue();
        msaa_render_targets_[0] = cgpu_create_texture(device_, &resolve_desc);
        CGPUTextureViewDescriptor resolve_view_desc = {};
        resolve_view_desc.texture = msaa_render_targets_[0];
        resolve_view_desc.format = resolve_desc.format;
        resolve_view_desc.array_layer_count = 1;
        resolve_view_desc.base_array_layer = 0;
        resolve_view_desc.mip_level_count = 1;
        resolve_view_desc.base_mip_level = 0;
        resolve_view_desc.aspects = CGPU_TVA_COLOR;
        resolve_view_desc.dims = resolve_desc.sample_count == CGPU_SAMPLE_COUNT_1 ? CGPU_TEX_DIMENSION_2D : CGPU_TEX_DIMENSION_2DMS;
        resolve_view_desc.usages = CGPU_TVU_RTV_DSV;
        msaa_render_target_views_[0] = cgpu_create_texture_view(device_, &resolve_view_desc);
        for (uint32_t i = 1; i < swapchain_->buffer_count; i++)
        {
            msaa_render_targets_[i] = msaa_render_targets_[0];
            msaa_render_target_views_[i] = msaa_render_target_views_[0];
        }
    }
    else
    {
        for (uint32_t i = 0; i < swapchain_->buffer_count; i++)
        {
            msaa_render_targets_[i] = swapchain_->back_buffers[i];
            msaa_render_target_views_[i] = views_[i];
        }
    }
    // create ds
    CGPUTextureDescriptor ds_desc = {};
    eastl::string name = "DepthStencil";
    ds_desc.name = name.c_str();
    ds_desc.descriptors = CGPU_RESOURCE_TYPE_TEXTURE | CGPU_RESOURCE_TYPE_DEPTH_STENCIL;
    ds_desc.flags = CGPU_TCF_OWN_MEMORY_BIT;
    ds_desc.width = swapchain_->back_buffers[0]->width;
    ds_desc.height = swapchain_->back_buffers[0]->height;
    ds_desc.depth = 1;
    ds_desc.format = CGPU_FORMAT_D32_SFLOAT;
    ds_desc.array_size = 1;
    ds_desc.sample_count = RenderWindow::SampleCount;
    ds_desc.start_state = CGPU_RESOURCE_STATE_DEPTH_WRITE;
    ds_desc.owner_queue = render_device->gfx_queue_;
    screen_ds_ = cgpu_create_texture(device_, &ds_desc);
    CGPUTextureViewDescriptor ds_view_desc = {};
    ds_view_desc.texture = screen_ds_;
    ds_view_desc.format = CGPU_FORMAT_D32_SFLOAT;
    ds_view_desc.array_layer_count = 1;
    ds_view_desc.base_array_layer = 0;
    ds_view_desc.mip_level_count = 1;
    ds_view_desc.base_mip_level = 0;
    ds_view_desc.aspects = CGPU_TVA_DEPTH;
    ds_view_desc.dims = ds_desc.sample_count == CGPU_SAMPLE_COUNT_1 ? CGPU_TEX_DIMENSION_2D : CGPU_TEX_DIMENSION_2DMS;
    ds_view_desc.usages = CGPU_TVU_RTV_DSV;
    screen_ds_view_ = cgpu_create_texture_view(device_, &ds_view_desc);
    for (uint32_t i = 0; i < swapchain_->buffer_count; i++)
        present_semaphores_[i] = render_device->AllocSemaphore();
}

CGPUSemaphoreId RenderWindow::AcquireNextFrame(uint32_t& backbuffer_index)
{
    present_semaphores_cursor_++;
    present_semaphores_cursor_ = present_semaphores_cursor_ % swapchain_->buffer_count;
    CGPUAcquireNextDescriptor acquire_desc = {};
    acquire_desc.signal_semaphore = present_semaphores_[present_semaphores_cursor_];
    backbuffer_index = cgpu_acquire_next_image(swapchain_, &acquire_desc);
    backbuffer_index_ = backbuffer_index;
    return acquire_desc.signal_semaphore;
}

void RenderWindow::BeginScreenPass(class RenderContext* ctx)
{
    const CGPUTextureId back_buffer = swapchain_->back_buffers[backbuffer_index_];
    const CGPUTextureId back_render_target = msaa_render_targets_[backbuffer_index_];
    CGPUColorAttachment screen_attachment = {};
    screen_attachment.view = msaa_render_target_views_[backbuffer_index_];
    screen_attachment.resolve_view = SampleCount == CGPU_SAMPLE_COUNT_1 ? nullptr : views_[backbuffer_index_];
    screen_attachment.load_action = CGPU_LOAD_ACTION_CLEAR;
    screen_attachment.store_action = SampleCount == CGPU_SAMPLE_COUNT_1 ? CGPU_STORE_ACTION_STORE : CGPU_STORE_ACTION_DISCARD;
    screen_attachment.clear_color = fastclear_0000;
    CGPUDepthStencilAttachment ds_attachment = {};
    ds_attachment.view = screen_ds_view_;
    ds_attachment.write_depth = true;
    ds_attachment.write_stencil = false;
    ds_attachment.clear_depth = 1.f;
    ds_attachment.depth_load_action = CGPU_LOAD_ACTION_CLEAR;
    ds_attachment.depth_store_action = CGPU_STORE_ACTION_DISCARD;
    ds_attachment.stencil_store_action = CGPU_STORE_ACTION_DISCARD;
    CGPURenderPassDescriptor rp_desc = {};
    rp_desc.render_target_count = 1;
    rp_desc.sample_count = RenderWindow::SampleCount;
    rp_desc.color_attachments = &screen_attachment;
    rp_desc.depth_stencil = &ds_attachment;
    CGPUTextureBarrier tex_barriers[2] = {};
    CGPUTextureBarrier& draw_barrier = tex_barriers[0];
    draw_barrier.texture = back_render_target;
    draw_barrier.src_state = CGPU_RESOURCE_STATE_UNDEFINED;
    draw_barrier.dst_state = CGPU_RESOURCE_STATE_RENDER_TARGET;
    CGPUTextureBarrier& resolve_barrier = tex_barriers[1];
    resolve_barrier.texture = back_buffer;
    resolve_barrier.src_state = CGPU_RESOURCE_STATE_UNDEFINED;
    resolve_barrier.dst_state = CGPU_RESOURCE_STATE_RESOLVE_DEST;
    CGPUResourceBarrierDescriptor barrier_desc0 = {};
    barrier_desc0.texture_barriers = SampleCount == CGPU_SAMPLE_COUNT_1 ? &draw_barrier : &resolve_barrier;
    barrier_desc0.texture_barriers_count = 1;
    ctx->ResourceBarrier(barrier_desc0);
    ctx->BeginRenderPass(rp_desc);
    // begin render scene
    ctx->SetViewport(0.0f, 0.0f,
    (float)back_buffer->width, (float)back_buffer->height,
    0.f, 1.f);
    ctx->SetScissor(0, 0, back_buffer->width, back_buffer->height);
}

void RenderWindow::EndScreenPass(class RenderContext* ctx)
{
    const CGPUTextureId back_buffer = swapchain_->back_buffers[backbuffer_index_];
    ctx->EndRenderPass();
    CGPUTextureBarrier present_barrier = {};
    present_barrier.texture = back_buffer;
    present_barrier.src_state = SampleCount == CGPU_SAMPLE_COUNT_1 ? CGPU_RESOURCE_STATE_RENDER_TARGET : CGPU_RESOURCE_STATE_RESOLVE_DEST;
    present_barrier.dst_state = CGPU_RESOURCE_STATE_PRESENT;
    CGPUResourceBarrierDescriptor barrier_desc1 = {};
    barrier_desc1.texture_barriers = &present_barrier;
    barrier_desc1.texture_barriers_count = 1;
    ctx->ResourceBarrier(barrier_desc1);
}

void RenderWindow::Present(uint32_t index, const CGPUSemaphoreId* wait_semaphores, uint32_t semaphore_count)
{
    eastl::vector<CGPUSemaphoreId> final_semaphores = {};
    final_semaphores.reserve(semaphore_count + 1);
    final_semaphores.emplace_back(present_semaphores_[present_semaphores_cursor_]);
    for (uint32_t i = 0; i < semaphore_count; i++)
    {
        final_semaphores.emplace_back(wait_semaphores[i]);
    }
    CGPUQueuePresentDescriptor present_desc = {};
    present_desc.index = index;
    present_desc.wait_semaphore_count = final_semaphores.size();
    present_desc.wait_semaphores = final_semaphores.data();
    present_desc.swapchain = swapchain_;
    cgpu_queue_present(render_device_->GetPresentQueue(), &present_desc);
}

void RenderWindow::Destroy()
{
    if (msaa_render_targets_[0] != swapchain_->back_buffers[0]) cgpu_free_texture(msaa_render_targets_[0]);
    if (msaa_render_target_views_[0] != views_[0]) cgpu_free_texture_view(msaa_render_target_views_[0]);
    for (uint32_t i = 0; i < 3; i++)
    {
        if (views_[i] != nullptr) cgpu_free_texture_view(views_[i]);
    }
    if (screen_ds_ != nullptr) cgpu_free_texture(screen_ds_);
    if (screen_ds_view_ != nullptr) cgpu_free_texture_view(screen_ds_view_);
    auto device = swapchain_->device;
    for (uint32_t i = 0; i < swapchain_->buffer_count; i++)
        render_device_->FreeSemaphore(present_semaphores_[i]);
    cgpu_free_swapchain(swapchain_);
    cgpu_free_surface(device, surface_);
    SDL_DestroyWindow(sdl_window_);
}

void RenderDevice::Initialize(ECGPUBackend backend, RenderWindow** pprender_window)
{
    // create backend device
    backend_ = backend;
    // create instance
    CGPUInstanceDescriptor instance_desc = {};
    instance_desc.backend = backend;
    instance_desc.enable_debug_layer = true;
    instance_desc.enable_gpu_based_validation = true;
    instance_desc.enable_set_name = true;
    instance_ = cgpu_create_instance(&instance_desc);
    {
        uint32_t adapters_count = 0;
        cgpu_enum_adapters(instance_, CGPU_NULLPTR, &adapters_count);
        CGPUAdapterId adapters[256];
        cgpu_enum_adapters(instance_, adapters, &adapters_count);
        adapter_ = adapters[0];
    }
    const auto cpy_queue_count_ = cgpu_query_queue_count(adapter_, QUEUE_TYPE_TRANSFER);
    CGPUQueueGroupDescriptor Gs[2];
    Gs[0].queueType = QUEUE_TYPE_GRAPHICS;
    Gs[0].queueCount = 1;
    Gs[1].queueType = QUEUE_TYPE_TRANSFER;
    Gs[1].queueCount = cgpu_min(cpy_queue_count_, MAX_CPY_QUEUE_COUNT);
    if (Gs[1].queueCount)
    {
        CGPUDeviceDescriptor device_desc = {};
        device_desc.queueGroups = Gs;
        device_desc.queueGroupCount = 2;
        device_ = cgpu_create_device(adapter_, &device_desc);
        gfx_queue_ = cgpu_get_queue(device_, QUEUE_TYPE_GRAPHICS, 0);
        cpy_queue_ = cgpu_get_queue(device_, QUEUE_TYPE_TRANSFER, 0);
        for (uint32_t i = 1; i < Gs[1].queueCount; i++)
        {
            extra_cpy_queues_[i - 1] = cgpu_get_queue(device_, QUEUE_TYPE_TRANSFER, i);
            extra_cpy_queue_count_++;
        }
    }
    else
    {
        CGPUDeviceDescriptor device_desc = {};
        device_desc.queueGroups = Gs;
        device_desc.queueGroupCount = 1;
        device_ = cgpu_create_device(adapter_, &device_desc);
        gfx_queue_ = cgpu_get_queue(device_, QUEUE_TYPE_GRAPHICS, 0);
        cpy_queue_ = gfx_queue_;
    }
    // copy cmd pool
    CGPUCommandPoolDescriptor pool_desc = {};
    cpy_cmd_pool_ = cgpu_create_command_pool(cpy_queue_, &pool_desc);

    *pprender_window = new RenderWindow();
    RenderWindow* render_window = *pprender_window;
    render_window->Initialize(this);
    screen_format_ = (ECGPUFormat)render_window->swapchain_->back_buffers[0]->format;

    // create default shaders & resources
    CGPUShaderLibraryDescriptor vs_desc = {};
    uint32_t *vs_bytes, *fs_bytes;
    read_shader_bytes("cgpu-3d/vertex_shader",
    &vs_bytes, &vs_desc.code_size, backend);
    vs_desc.name = "VertexShaderLibrary";
    vs_desc.stage = CGPU_SHADER_STAGE_VERT;
    vs_desc.code = vs_bytes;
    vs_library_ = cgpu_create_shader_library(device_, &vs_desc);
    CGPUShaderLibraryDescriptor fs_desc = {};
    read_shader_bytes("cgpu-3d/fragment_shader",
    &fs_bytes, &fs_desc.code_size, backend);
    fs_desc.name = "FragmentShaderLibrary";
    fs_desc.stage = CGPU_SHADER_STAGE_FRAG;
    fs_desc.code = fs_bytes;
    fs_library_ = cgpu_create_shader_library(device_, &fs_desc);
    ::free((void*)vs_desc.code);
    ::free((void*)fs_desc.code);
    CGPUSamplerDescriptor sampler_desc = {};
    sampler_desc.address_u = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.address_v = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.address_w = CGPU_ADDRESS_MODE_REPEAT;
    sampler_desc.mipmap_mode = CGPU_MIPMAP_MODE_LINEAR;
    sampler_desc.min_filter = CGPU_FILTER_TYPE_LINEAR;
    sampler_desc.mag_filter = CGPU_FILTER_TYPE_LINEAR;
    sampler_desc.compare_func = CGPU_CMP_NEVER;
    default_sampler_ = cgpu_create_sampler(device_, &sampler_desc);
    // create root signature
    ppl_shaders_[0].stage = CGPU_SHADER_STAGE_VERT;
    ppl_shaders_[0].entry = "main";
    ppl_shaders_[0].library = vs_library_;
    ppl_shaders_[1].stage = CGPU_SHADER_STAGE_FRAG;
    ppl_shaders_[1].entry = "main";
    ppl_shaders_[1].library = fs_library_;
    const char8_t* sampler_name = "texture_sampler";
    const char8_t* root_constant_name = "root_constants";
    CGPURootSignatureDescriptor rs_desc = {};
    rs_desc.shaders = ppl_shaders_;
    rs_desc.shader_count = 2;
    rs_desc.root_constant_names = &root_constant_name;
    rs_desc.root_constant_count = 1;
    rs_desc.static_samplers = &default_sampler_;
    rs_desc.static_sampler_count = 1;
    rs_desc.static_sampler_names = &sampler_name;
    root_sig_ = cgpu_create_root_signature(device_, &rs_desc);
}

void RenderDevice::Destroy()
{
    cgpu_free_sampler(default_sampler_);
    cgpu_free_root_signature(root_sig_);
    cgpu_free_shader_library(vs_library_);
    cgpu_free_shader_library(fs_library_);
    cgpu_free_command_pool(cpy_cmd_pool_);
    cgpu_free_queue(gfx_queue_);
    if (cpy_queue_ != gfx_queue_)
        cgpu_free_queue(cpy_queue_);
    for (uint32_t i = 0; i < 7; i++)
    {
        if (extra_cpy_queues_[i]) cgpu_free_queue(extra_cpy_queues_[i]);
    }
    cgpu_free_device(device_);
    cgpu_free_instance(instance_);
}

void RenderDevice::FreeSemaphore(CGPUSemaphoreId semaphore)
{
    cgpu_free_semaphore(semaphore);
}

CGPUSemaphoreId RenderDevice::AllocSemaphore()
{
    return cgpu_create_semaphore(device_);
}

CGPUFenceId RenderDevice::AllocFence()
{
    return cgpu_create_fence(device_);
}

void RenderDevice::FreeFence(CGPUFenceId fence)
{
    cgpu_wait_fences(&fence, 1);
    cgpu_free_fence(fence);
}

CGPUDescriptorSetId RenderDevice::CreateDescriptorSet(const CGPURootSignatureId signature, uint32_t set_index)
{
    CGPUDescriptorSetDescriptor desc_desc = {};
    desc_desc.root_signature = signature;
    desc_desc.set_index = set_index;
    auto desc_set = cgpu_create_descriptor_set(device_, &desc_desc);
    CGPUDescriptorData arguments[1];
    arguments[0].name = "sampled_texture";
    arguments[0].count = 1;
    auto default_tex_view = RenderBlackboard::GetTexture("DefaultTexture")->view_;
    arguments[0].textures = &default_tex_view;
    cgpu_update_descriptor_set(desc_set, arguments, 1);
    return desc_set;
}

void RenderDevice::FreeDescriptorSet(CGPUDescriptorSetId desc_set)
{
    cgpu_free_descriptor_set(desc_set);
}

void RenderDevice::Submit(class RenderContext* context)
{
    CGPUQueueSubmitDescriptor submit_desc = {};
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

void RenderDevice::freeRenderPipeline(CGPURenderPipelineId pipeline)
{
    cgpu_free_render_pipeline(pipeline);
}

void GfxContext::Initialize(RenderDevice* device)
{
    device_ = device;
    CGPUCommandPoolDescriptor pool_desc = {};
    cmd_pool_ = cgpu_create_command_pool(device->GetGraphicsQueue(), &pool_desc);
    CGPUCommandBufferDescriptor cmd_desc = {};
    cmd_desc.is_secondary = false;
    cmd_buffer_ = cgpu_create_command_buffer(cmd_pool_, &cmd_desc);
    exec_fence_ = cgpu_create_fence(device->GetCGPUDevice());
}

void GfxContext::Begin()
{
    cgpu_wait_fences(&exec_fence_, 1);
    cgpu_reset_command_pool(cmd_pool_);
    cgpu_cmd_begin(cmd_buffer_);
}

void GfxContext::ResourceBarrier(const CGPUResourceBarrierDescriptor& barrier_desc)
{
    cgpu_cmd_resource_barrier(cmd_buffer_, &barrier_desc);
}

void GfxContext::AcquireResources(AsyncRenderTexture* const* textures, uint32_t textures_count,
AsyncRenderBuffer* const* buffers, uint32_t buffers_count)
{
    const auto theQueueType = QUEUE_TYPE_GRAPHICS;
    eastl::vector<CGPUTextureBarrier> tex_barriers(textures_count);
    eastl::vector<CGPUBufferBarrier> buf_barriers(buffers_count);
    for (uint32_t i = 0; i < textures_count; i++)
    {
        tex_barriers[i].texture = textures[i]->texture_;
        tex_barriers[i].src_state = CGPU_RESOURCE_STATE_COPY_DEST;
        tex_barriers[i].dst_state = CGPU_RESOURCE_STATE_SHADER_RESOURCE;
        if (textures[i]->queue_type_.load() == QUEUE_TYPE_TRANSFER)
        {
            tex_barriers[i].queue_acquire = true;
            tex_barriers[i].queue_type = theQueueType;
        }
        textures[i]->queue_type_ = theQueueType;
        textures[i]->queue_released_ = false;
    }
    for (uint32_t i = 0; i < buffers_count; i++)
    {
        buf_barriers[i].buffer = buffers[i]->buffer_;
        buf_barriers[i].src_state = CGPU_RESOURCE_STATE_COPY_DEST;
        buf_barriers[i].dst_state = CGPU_RESOURCE_STATE_SHADER_RESOURCE;
        if (textures[i]->queue_type_.load() == QUEUE_TYPE_TRANSFER)
        {
            tex_barriers[i].queue_acquire = true;
            buf_barriers[i].queue_type = theQueueType;
        }
        buffers[i]->queue_type_ = theQueueType;
        buffers[i]->queue_released_ = false;
    }
    CGPUResourceBarrierDescriptor acquire_barriers = {};
    acquire_barriers.texture_barriers = tex_barriers.data();
    acquire_barriers.texture_barriers_count = (uint32_t)tex_barriers.size();
    acquire_barriers.buffer_barriers = buf_barriers.data();
    acquire_barriers.buffer_barriers_count = (uint32_t)buf_barriers.size();
    ResourceBarrier(acquire_barriers);
}

void GfxContext::End()
{
    cgpu_cmd_end(cmd_buffer_);
}

void GfxContext::Wait()
{
    if (cgpu_query_fence_status(exec_fence_) == CGPU_FENCE_STATUS_INCOMPLETE)
        cgpu_wait_fences(&exec_fence_, 1);
}

void GfxContext::Destroy()
{
    auto status = cgpu_query_fence_status(exec_fence_);
    if (status == CGPU_FENCE_STATUS_INCOMPLETE)
    {
        cgpu_wait_fences(&exec_fence_, 1);
    }
    cgpu_free_fence(exec_fence_);
    cgpu_reset_command_pool(cmd_pool_);
    cgpu_free_command_buffer(cmd_buffer_);
    cgpu_free_command_pool(cmd_pool_);
}

void RenderContext::BeginRenderPass(const CGPURenderPassDescriptor& rp_desc)
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

void RenderContext::BindPipeline(CGPURenderPipelineId pipeline)
{
    if (cached_pipeline != pipeline)
    {
        cgpu_render_encoder_bind_pipeline(rp_encoder_, pipeline);
        cached_pipeline = pipeline;
    }
}

void RenderContext::BindVertexBuffers(uint32_t buffer_count, const CGPUBufferId* buffers, const uint32_t* strides, const uint32_t* offsets)
{
    cgpu_render_encoder_bind_vertex_buffers(rp_encoder_, buffer_count, buffers, strides, offsets);
}

void RenderContext::BindIndexBuffer(CGPUBufferId buffer, uint32_t index_stride, uint64_t offset)
{
    cgpu_render_encoder_bind_index_buffer(rp_encoder_, buffer, index_stride, offset);
}

void RenderContext::BindDescriptorSet(CGPUDescriptorSetId desc_set)
{
    if (cached_descset != desc_set)
    {
        cgpu_render_encoder_bind_descriptor_set(rp_encoder_, desc_set);
        cached_descset = desc_set;
    }
}

void RenderContext::PushConstants(CGPURootSignatureId rs, const char8_t* name, const void* data)
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
    cached_pipeline = nullptr;
    cached_descset = nullptr;
}