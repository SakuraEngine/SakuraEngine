#include "render-device.hpp"
#include "render-resources.hpp"
#include "thirdparty/lodepng.h"
#include "math/scalarmath.h"

// Aux Thread
void loaderFunction(void* data)
{
    RenderAuxThread* dt = (RenderAuxThread*)data;
    while (dt->is_running_)
    {
        skr_acquire_mutex(&dt->load_mutex_);
        auto tasks = dt->task_queue_;
        dt->task_queue_.clear();
        skr_release_mutex(&dt->load_mutex_);
        for (auto&& task : tasks)
        {
            task.first(dt->render_device_->GetCGPUDevice());
            task.second();
        }
        skr_thread_sleep(1);
    }
}
void RenderAuxThread::Enqueue(const AuxThreadTaskWithCallback& task)
{
    if (!force_block_)
        task_queue_.emplace_back(task);
    else
    {
        task.first(render_device_->GetCGPUDevice());
        task.second();
    }
}

void RenderAuxThread::Initialize(class RenderDevice* render_device)
{
    render_device_ = render_device;
    is_running_ = true;
    aux_item_.pData = this;
    aux_item_.pFunc = &loaderFunction;
    skr_init_thread(&aux_item_, &aux_thread_);
    skr_init_mutex(&load_mutex_);
}

void RenderAuxThread::Wait()
{
    if (is_running_)
    {
        is_running_ = false;
        skr_join_thread(aux_thread_);
    }
}

void RenderAuxThread::Destroy()
{
    Wait();
    skr_destroy_mutex(&load_mutex_);
    skr_destroy_thread(aux_thread_);
}

void AsyncTransferThread::Initialize(class RenderDevice* render_device)
{
    render_device_ = render_device;
    // copy cmd pool
    CGpuCommandPoolDescriptor pool_desc = {};
    cpy_cmd_pool_ = cgpu_create_command_pool(render_device->cpy_queue_, &pool_desc);
}

void AsyncTransferThread::Destroy()
{
    for (auto&& iter : async_cpy_cmds_)
    {
        if (iter.first) render_device_->FreeFence(iter.first);
        cgpu_free_command_buffer(iter.second);
    }
    cgpu_free_command_pool(cpy_cmd_pool_);
}

void AsyncTransferThread::asyncTransfer(const AsyncBufferToBufferTransfer* transfers,
    uint32_t transfer_count, CGpuSemaphoreId semaphore, CGpuFenceId fence)
{
    CGpuCommandBufferDescriptor cmd_desc = {};
    CGpuCommandBufferId cmd = cgpu_create_command_buffer(cpy_cmd_pool_, &cmd_desc);
    cgpu_cmd_begin(cmd);
    for (uint32_t i = 0; i < transfer_count; i++)
    {
        CGpuBufferToBufferTransfer trans = {};
        trans.dst = transfers[i].dst->buffer_;
        trans.dst_offset = transfers[i].dst_offset;
        trans.src = transfers[i].src ? transfers[i].src->buffer_ : transfers[i].raw_src;
        trans.src_offset = transfers[i].src_offset;
        trans.size = transfers[i].size;
        cgpu_cmd_transfer_buffer_to_buffer(cmd, &trans);
    }
    const auto asyncCopy = render_device_->AsyncCopyQueueEnabled();
    cgpu_cmd_end(cmd);
    CGpuQueueSubmitDescriptor submit_desc = {};
    submit_desc.cmds = &cmd;
    submit_desc.cmds_count = 1;
    submit_desc.signal_semaphore_count = semaphore ? 1 : 0;
    submit_desc.signal_semaphores = semaphore ? &semaphore : nullptr;
    submit_desc.signal_fence = fence;
    cgpu_submit_queue(render_device_->GetAsyncCopyQueue(), &submit_desc);
    for (uint32_t i = 0; i < transfer_count; i++)
    {
        transfers[i].dst->upload_started_ = true;
        transfers[i].dst->queue_released_ = false;
        transfers[i].dst->queue_type_ = asyncCopy ? QUEUE_TYPE_TRANSFER : QUEUE_TYPE_GRAPHICS;
    }
    if (fence)
        async_cpy_cmds_[fence] = cmd;
}

void AsyncTransferThread::asyncTransfer(const AsyncBufferToTextureTransfer* transfers,
    uint32_t transfer_count, CGpuSemaphoreId semaphore, CGpuFenceId fence)
{
    CGpuCommandBufferDescriptor cmd_desc = {};
    CGpuCommandBufferId cmd = cgpu_create_command_buffer(cpy_cmd_pool_, &cmd_desc);
    cgpu_cmd_begin(cmd);
    for (uint32_t i = 0; i < transfer_count; i++)
    {
        CGpuBufferToTextureTransfer trans = {};
        trans.dst = transfers[i].dst->texture_;
        trans.dst_mip_level = transfers[i].dst_mip_level;
        trans.elems_per_row = transfers[i].elems_per_row;
        trans.rows_per_image = transfers[i].rows_per_image;
        trans.base_array_layer = transfers[i].base_array_layer;
        trans.layer_count = transfers[i].layer_count;
        trans.src = transfers[i].src ? transfers[i].src->buffer_ : transfers[i].raw_src;
        trans.src_offset = transfers[i].src_offset;
        cgpu_cmd_transfer_buffer_to_texture(cmd, &trans);
    }
    const auto asyncCopy = render_device_->AsyncCopyQueueEnabled();
    cgpu_cmd_end(cmd);
    CGpuQueueSubmitDescriptor submit_desc = {};
    submit_desc.cmds = &cmd;
    submit_desc.cmds_count = 1;
    submit_desc.signal_semaphore_count = semaphore ? 1 : 0;
    submit_desc.signal_semaphores = semaphore ? &semaphore : nullptr;
    submit_desc.signal_fence = fence;
    cgpu_submit_queue(render_device_->GetAsyncCopyQueue(), &submit_desc);
    for (uint32_t i = 0; i < transfer_count; i++)
    {
        transfers[i].dst->upload_started_ = true;
        transfers[i].dst->queue_released_ = false;
        transfers[i].dst->queue_type_ = asyncCopy ? QUEUE_TYPE_TRANSFER : QUEUE_TYPE_GRAPHICS;
    }
    if (fence)
        async_cpy_cmds_[fence] = cmd;
}

AsyncRenderTexture* AsyncTransferThread::UploadTexture(AsyncRenderTexture* target, const void* data, size_t data_size, CGpuFenceId fence)
{
    if (target == nullptr)
        return target;
    const void* src_ptr = data;
    size_t row_data_size = 0;
    size_t upload_row_pitch = 0;
    row_data_size = target->texture_->width *
                    FormatUtil_BitSizeOfBlock((ECGpuFormat)target->texture_->format) / 8;
    upload_row_pitch = smath_round_up(row_data_size, 256);
    if (target->image_bytes_ != nullptr)
    {
        src_ptr = target->image_bytes_;
    }
    // upload texture data
    CGpuBufferDescriptor upload_buffer_desc = {};
    upload_buffer_desc.name = eastl::string("Upload-Texture").c_str();
    upload_buffer_desc.flags = BCF_OWN_MEMORY_BIT | BCF_PERSISTENT_MAP_BIT;
    upload_buffer_desc.descriptors = RT_NONE;
    upload_buffer_desc.memory_usage = MEM_USAGE_CPU_ONLY;
    if (!target->upload_buffer_)
    {
        // fuck d3d
        if (target->texture_->device->adapter->instance->backend == CGPU_BACKEND_D3D12)
            upload_buffer_desc.size = upload_row_pitch * target->texture_->height;
        else
            upload_buffer_desc.size = data_size;
        target->upload_buffer_ = cgpu_create_buffer(render_device_->GetCGPUDevice(), &upload_buffer_desc);
    }
    // upload texture
    if (target->texture_->device->adapter->instance->backend == CGPU_BACKEND_D3D12)
    {
        for (uint32_t i = 0; i < target->texture_->height; i++)
        {
            const auto row_start =
                (uint8_t*)target->upload_buffer_->cpu_mapped_address + i * upload_row_pitch;
            const auto src_row_start = (uint8_t*)src_ptr + i * row_data_size;
            memcpy(row_start, src_row_start, row_data_size);
        }
    }
    else
    {
        memcpy(target->upload_buffer_->cpu_mapped_address, data, data_size);
    }
    CGpuCommandBufferDescriptor cmd_desc = {};
    cmd_desc.is_secondary = false;
    CGpuCommandBufferId cmd = cgpu_create_command_buffer(render_device_->cpy_cmd_pool_, &cmd_desc);
    cgpu_cmd_begin(cmd);
    CGpuBufferToTextureTransfer b2t = {};
    b2t.src = target->upload_buffer_;
    b2t.src_offset = 0;
    b2t.dst = target->texture_;
    b2t.elems_per_row = target->texture_->width;
    b2t.rows_per_image = target->texture_->height;
    b2t.base_array_layer = 0;
    b2t.layer_count = 1;
    cgpu_cmd_transfer_buffer_to_texture(cmd, &b2t);
    cgpu_cmd_end(cmd);
    CGpuQueueSubmitDescriptor cpy_submit = {};
    cpy_submit.cmds = &cmd;
    cpy_submit.cmds_count = 1;
    cpy_submit.signal_fence = fence ? fence : nullptr;
    cgpu_submit_queue(render_device_->GetAsyncCopyQueue(), &cpy_submit);
    if (fence)
        async_cpy_cmds_[fence] = cmd;
    target->upload_started_ = true;
    target->queue_released_ = false;
    const auto asyncCopy = render_device_->AsyncCopyQueueEnabled();
    target->queue_type_ = asyncCopy ? QUEUE_TYPE_TRANSFER : QUEUE_TYPE_GRAPHICS;
    return target;
}

// Render Buffer
void AsyncRenderBuffer::Initialize(struct RenderAuxThread* aux_thread, const CGpuBufferDescriptor& buff_desc, const AuxTaskCallback& cb)
{
    aux_thread->Enqueue({ [=](CGpuDeviceId device) {
                             buffer_ = cgpu_create_buffer(device, &buff_desc);
                             resource_handle_ready_ = true;
                         },
        cb });
}

void AsyncRenderBuffer::Destroy(struct RenderAuxThread* aux_thread, const AuxTaskCallback& cb)
{
    auto destructor = [this](CGpuDeviceId device) {
        cgpu_free_buffer(buffer_);
        buffer_ = nullptr;
    };
    if (aux_thread)
        aux_thread->Enqueue({ destructor, cb });
    else
    {
        destructor(buffer_->device);
        cb();
    }
}

// Render Texture
void AsyncRenderTexture::Initialize(struct RenderAuxThread* aux_thread,
    const CGpuTextureDescriptor& tex_desc, const AuxTaskCallback& cb, bool default_srv)
{
    aux_thread->Enqueue(
        { [=](CGpuDeviceId device) {
             texture_ = cgpu_create_texture(device, &tex_desc);
             if (default_srv)
             {
                 CGpuTextureViewDescriptor view_desc = {};
                 eastl::string view_name = tex_desc.name;
                 view_name.append("-view");
                 view_desc.name = view_name.c_str();
                 view_desc.texture = texture_;
                 view_desc.array_layer_count = tex_desc.array_size;
                 view_desc.base_array_layer = 0;
                 view_desc.base_mip_level = 0;
                 view_desc.mip_level_count = tex_desc.mip_levels;
                 view_desc.aspects = TVA_COLOR;
                 view_desc.dims = TEX_DIMENSION_2D;
                 view_desc.format = tex_desc.format;
                 view_desc.usages = TVU_SRV;
                 view_ = cgpu_create_texture_view(device, &view_desc);
             }
             resource_handle_ready_ = true;
         },
            cb });
}

eastl::cached_hashset<CGpuVertexLayout> RenderBlackboard::vertex_layouts_;
eastl::vector_map<eastl::string, AsyncRenderTexture*> RenderBlackboard::textures_;
eastl::unordered_map<PipelineKey, AsyncRenderPipeline*> RenderBlackboard::pipelines_;
void AsyncRenderTexture::Initialize(struct RenderAuxThread* aux_thread, const eastl::string name,
    const eastl::string disk_file, ECGpuFormat format, const AuxTaskCallback& cb)
{
    aux_thread->Enqueue(
        { [=](CGpuDeviceId device) {
             unsigned width, height;
             ::lodepng_decode32_file(&this->image_bytes_, &width, &height, disk_file.c_str());

             CGpuTextureDescriptor tex_desc = {};
             tex_desc.name = name.c_str();
             tex_desc.descriptors = RT_TEXTURE;
             tex_desc.flags = TCF_OWN_MEMORY_BIT;
             tex_desc.width = width;
             tex_desc.height = height;
             tex_desc.depth = 1;
             tex_desc.format = format;
             tex_desc.array_size = 1;
             tex_desc.start_state = RESOURCE_STATE_COPY_DEST;
             tex_desc.owner_queue = aux_thread->render_device_->GetCopyQueue();
             texture_ = cgpu_create_texture(device, &tex_desc);

             CGpuTextureViewDescriptor sview_desc = {};
             sview_desc.format = tex_desc.format;
             sview_desc.array_layer_count = 1;
             sview_desc.base_array_layer = 0;
             sview_desc.mip_level_count = 1;
             sview_desc.base_mip_level = 0;
             sview_desc.aspects = TVA_COLOR;
             sview_desc.dims = TEX_DIMENSION_2D;
             sview_desc.usages = TVU_SRV;
             sview_desc.texture = texture_;
             view_ = cgpu_create_texture_view(device, &sview_desc);

             {
                 auto data_size = texture_->width * texture_->height *
                                  FormatUtil_BitSizeOfBlock((ECGpuFormat)texture_->format) / 8;
                 CGpuBufferDescriptor upload_buffer_desc = {};
                 auto upload_name = eastl::string("Upload-").append(tex_desc.name ? tex_desc.name : "");
                 upload_buffer_desc.name = upload_name.c_str(),
                 upload_buffer_desc.flags = BCF_OWN_MEMORY_BIT | BCF_PERSISTENT_MAP_BIT,
                 upload_buffer_desc.descriptors = RT_NONE,
                 upload_buffer_desc.memory_usage = MEM_USAGE_CPU_ONLY,
                 upload_buffer_desc.size = data_size;
                 upload_buffer_ = cgpu_create_buffer(device, &upload_buffer_desc);
                 // upload texture
                 {
                     memcpy(upload_buffer_->cpu_mapped_address, image_bytes_, upload_buffer_desc.size);
                 }
             }

             resource_handle_ready_ = true;
         },
            cb });
}

void AsyncRenderTexture::Initialize(struct RenderAuxThread* aux_thread,
    const CGpuTextureDescriptor& tex_desc, const CGpuTextureViewDescriptor& tex_view_desc, const AuxTaskCallback& cb)
{
    aux_thread->Enqueue(
        { [=](CGpuDeviceId device) {
             texture_ = cgpu_create_texture(device, &tex_desc);
             CGpuTextureViewDescriptor view_desc = tex_view_desc;
             view_desc.texture = texture_;
             view_ = cgpu_create_texture_view(device, &view_desc);

             resource_handle_ready_ = true;
         },
            cb });
}

void AsyncRenderTexture::Destroy(struct RenderAuxThread* aux_thread, const AuxTaskCallback& cb)
{
    auto destructor = [this](CGpuDeviceId device) {
        if (image_bytes_) free(image_bytes_);
        cgpu_free_texture(texture_);
        if (view_) cgpu_free_texture_view(view_);
        if (upload_buffer_) cgpu_free_buffer(upload_buffer_);
        texture_ = nullptr;
        view_ = nullptr;
        upload_buffer_ = nullptr;
    };
    if (aux_thread)
        aux_thread->Enqueue({ destructor, cb });
    else
    {
        destructor(texture_->device);
        cb();
    }
}

void AsyncRenderShader::Initialize(struct RenderAuxThread* aux_thread, const CGpuShaderLibraryDescriptor& desc, const AuxTaskCallback& cb)
{
    aux_thread->Enqueue({ [=](CGpuDeviceId device) {
                             shader_ = cgpu_create_shader_library(device, &desc);
                             resource_handle_ready_ = true;
                         },
        cb });
}

void AsyncRenderShader::Destroy(struct RenderAuxThread* aux_thread, const AuxTaskCallback& cb)
{
    auto destructor = [this](CGpuDeviceId device) {
        cgpu_free_shader_library(shader_);
        shader_ = nullptr;
    };
    if (aux_thread)
        aux_thread->Enqueue({ destructor, cb });
    else
    {
        destructor(shader_->device);
        cb();
    }
}

void AsyncRenderPipeline::Initialize(struct RenderAuxThread* aux_thread, const CGpuRenderPipelineDescriptor& desc, const AuxTaskCallback& cb)
{
    aux_thread->Enqueue({ [=](CGpuDeviceId device) {
                             pipeline_ = cgpu_create_render_pipeline(device, &desc);
                             CGpuDescriptorSetDescriptor desc_desc = {};
                             desc_desc.root_signature = desc.root_signature;
                             desc_desc.set_index = 0;
                             desc_set_ = aux_thread->render_device_->CreateDescriptorSet(desc.root_signature, 0);
                             resource_handle_ready_ = true;
                         },
        cb });
}

void AsyncRenderPipeline::Destroy(struct RenderAuxThread* aux_thread, const AuxTaskCallback& cb)
{
    auto destructor = [this](CGpuDeviceId device) {
        cgpu_free_render_pipeline(pipeline_);
        cgpu_free_descriptor_set(desc_set_);
        desc_set_ = nullptr;
        pipeline_ = nullptr;
    };
    if (aux_thread)
        aux_thread->Enqueue({ destructor, cb });
    else
    {
        destructor(pipeline_->device);
        cb();
    }
}

void RenderBlackboard::Initialize()
{
}

AsyncRenderTexture* RenderBlackboard::GetTexture(const char* name)
{
    auto&& iter = textures_.find(name);
    if (iter != textures_.end())
    {
        return iter->second;
    }
    return nullptr;
}

AsyncRenderTexture* RenderBlackboard::AddTexture(const char* name, const char* disk_file,
    struct RenderAuxThread* aux_thread, ECGpuFormat format, const AuxTaskCallback& cb)
{
    if (GetTexture(name) != nullptr)
        return GetTexture(name);
    textures_.reserve(textures_.size() + 1);
    auto&& target = textures_.emplace_back(name, new AsyncRenderTexture());
    target.second->Initialize(aux_thread, name, disk_file, format, cb);
    return target.second;
}

AsyncRenderTexture* RenderBlackboard::AddTexture(const char* name, struct RenderAuxThread* aux_thread,
    uint32_t width, uint32_t height, ECGpuFormat format, const AuxTaskCallback& cb)
{
    if (GetTexture(name) != nullptr)
        return GetTexture(name);
    CGpuTextureDescriptor tex_desc = {};
    tex_desc.name = name;
    tex_desc.descriptors = RT_TEXTURE;
    tex_desc.flags = TCF_OWN_MEMORY_BIT;
    tex_desc.width = width;
    tex_desc.height = height;
    tex_desc.depth = 1;
    tex_desc.format = format;
    tex_desc.array_size = 1;
    tex_desc.start_state = RESOURCE_STATE_COPY_DEST;
    tex_desc.owner_queue = aux_thread->render_device_->cpy_queue_;
    CGpuTextureViewDescriptor sview_desc = {};
    sview_desc.format = tex_desc.format;
    sview_desc.array_layer_count = 1;
    sview_desc.base_array_layer = 0;
    sview_desc.mip_level_count = 1;
    sview_desc.base_mip_level = 0;
    sview_desc.aspects = TVA_COLOR;
    sview_desc.dims = TEX_DIMENSION_2D;
    sview_desc.usages = TVU_SRV;
    textures_.reserve(textures_.size() + 1);
    auto&& target = textures_.emplace_back(name, new AsyncRenderTexture());
    target.second->Initialize(aux_thread, tex_desc, sview_desc, cb);
    return target.second;
}

AsyncRenderPipeline* RenderBlackboard::GetRenderPipeline(const PipelineKey& key)
{
    auto&& iter = pipelines_.find(key);
    if (iter != pipelines_.end()) return iter->second;
    return nullptr;
}

AsyncRenderPipeline* RenderBlackboard::AddRenderPipeline(RenderAuxThread* aux_thread, const PipelineKey& key, const AuxTaskCallback& cb)
{
    {
        auto found = GetRenderPipeline(key);
        if (found != nullptr)
            return found;
    }
    pipelines_[key] = new AsyncRenderPipeline();
    auto ppl = GetRenderPipeline(key);
    auto descHeap = new eastl::tuple<
        CGpuRasterizerStateDescriptor,
        CGpuRenderPipelineDescriptor,
        CGpuDepthStateDescriptor,
        ECGpuFormat>();
    auto&& rasterizer_state = eastl::get<CGpuRasterizerStateDescriptor>(*descHeap);
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
    auto&& rp_desc = eastl::get<CGpuRenderPipelineDescriptor>(*descHeap);
    rp_desc.root_signature = key.root_sig_;
    rp_desc.prim_topology = PRIM_TOPO_TRI_LIST;
    auto viter = RenderBlackboard::GetVertexLayouts()->find_by_hash(key.vertex_layout_id_);
    rp_desc.vertex_layout = &viter.get_node()->mValue;
    rp_desc.vertex_shader = &aux_thread->render_device_->ppl_shaders_[0];
    rp_desc.fragment_shader = &aux_thread->render_device_->ppl_shaders_[1];
    rp_desc.rasterizer_state = &rasterizer_state;
    auto&& ds_state = eastl::get<CGpuDepthStateDescriptor>(*descHeap);
    ds_state.depth_write = true;
    ds_state.depth_test = true;
    ds_state.depth_func = CMP_GEQUAL;
    ds_state.stencil_test = false;
    rp_desc.depth_stencil_format = PF_D24_UNORM_S8_UINT;
    rp_desc.depth_state = &ds_state;
    rp_desc.render_target_count = 1;
    auto&& color_format = eastl::get<ECGpuFormat>(*descHeap);
    color_format = key.screen_format_;
    rp_desc.color_formats = &color_format;
    rp_desc.sample_count = key.sample_count_;
    ppl->Initialize(aux_thread, rp_desc, [descHeap]() {
        delete descHeap;
    });
    return ppl;
}

void RenderBlackboard::Finalize(struct RenderAuxThread* aux_thread)
{
    for (auto&& iter : textures_)
    {
        iter.second->Destroy(aux_thread);
        delete iter.second;
    }
    for (auto&& iter : pipelines_)
    {
        iter.second->Destroy(aux_thread);
        delete iter.second;
    }
}
