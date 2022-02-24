#include "render-device.hpp"
#include "render-resources.hpp"

// Aux Thread
void loaderFunction(void* data)
{
    RenderAuxThread* dt = (RenderAuxThread*)data;
    while (dt->is_running_)
    {
        skr_acquire_mutex(&dt->load_mutex_);
        auto tasks = dt->task_queue_;
        auto tasks2 = dt->task_queue2_;
        dt->task_queue_.clear();
        dt->task_queue2_.clear();
        skr_release_mutex(&dt->load_mutex_);
        for (auto&& task : tasks)
        {
            task(dt->render_device_->GetCGPUDevice());
        }
        for (auto&& task : tasks2)
        {
            task.first(dt->render_device_->GetCGPUDevice());
            task.second();
        }
        skr_thread_sleep(1);
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

void RenderAuxThread::Enqueue(const AuxThreadTask& task)
{
    task_queue_.emplace_back(task);
}

void RenderAuxThread::Enqueue(const AuxThreadTaskWithCallback& task)
{
    task_queue2_.emplace_back(task);
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
    aux_thread->Enqueue({ [=](CGpuDeviceId device) {
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

void AsyncRenderTexture::Initialize(struct RenderAuxThread* aux_thread,
    const CGpuTextureDescriptor& tex_desc, const CGpuTextureViewDescriptor& tex_view_desc, const AuxTaskCallback& cb)
{
    aux_thread->Enqueue({ [=](CGpuDeviceId device) {
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
        cgpu_free_texture(texture_);
        if (view_) cgpu_free_texture_view(view_);
        texture_ = nullptr;
        view_ = nullptr;
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
                             resource_handle_ready_ = true;
                         },
        cb });
}

void AsyncRenderPipeline::Destroy(struct RenderAuxThread* aux_thread, const AuxTaskCallback& cb)
{
    auto destructor = [this](CGpuDeviceId device) {
        cgpu_free_render_pipeline(pipeline_);
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

void RenderBlackboard::Finalize(struct RenderAuxThread* aux_thread)
{
    for (auto&& iter : textures_)
    {
        iter.second.Destroy(aux_thread);
    }
    for (auto&& iter : pipelines_)
    {
        iter.second.Destroy(aux_thread);
    }
}
