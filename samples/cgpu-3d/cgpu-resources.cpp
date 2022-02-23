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
        dt->task_queue_.clear();
        skr_release_mutex(&dt->load_mutex_);

        for (auto&& task : tasks)
        {
            task(dt->device_);
        }
        skr_thread_sleep(1);
    }
}

void RenderAuxThread::Initialize(class RenderDevice* render_device)
{
    device_ = render_device->GetCGPUDevice();
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

void RenderAuxThread::Wait()
{
    skr_join_thread(aux_thread_);
}

void RenderAuxThread::Destroy()
{
    is_running_ = false;
    Wait();
    skr_destroy_mutex(&load_mutex_);
    skr_destroy_thread(aux_thread_);
}

// Render Buffer
void RenderBuffer::Initialize(struct RenderAuxThread* aux_thread, const CGpuBufferDescriptor& buff_desc)
{
    aux_thread->Enqueue([=](CGpuDeviceId device) {
        buffer_ = cgpu_create_buffer(device, &buff_desc);
        upload_ready_fence_ = cgpu_create_fence(device);
        upload_ready_semaphore = cgpu_create_semaphore(device);
        resource_handle_ready_ = true;
    });
}

void RenderBuffer::Destroy(struct RenderAuxThread* aux_thread)
{
    aux_thread->Enqueue([this](CGpuDeviceId device) {
        cgpu_free_buffer(buffer_);
        cgpu_free_fence(upload_ready_fence_);
        cgpu_free_semaphore(upload_ready_semaphore);
        buffer_ = nullptr;
        upload_ready_fence_ = nullptr;
        upload_ready_semaphore = nullptr;
    });
}

// Render Texture
void RenderTexture::Initialize(struct RenderAuxThread* aux_thread,
    const CGpuTextureDescriptor& tex_desc, bool default_srv)
{
    aux_thread->Enqueue([=](CGpuDeviceId device) {
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
        upload_ready_fence_ = cgpu_create_fence(device);
        upload_ready_semaphore = cgpu_create_semaphore(device);
        resource_handle_ready_ = true;
    });
}

void RenderTexture::Initialize(struct RenderAuxThread* aux_thread,
    const CGpuTextureDescriptor& tex_desc, const CGpuTextureViewDescriptor& tex_view_desc)
{
    aux_thread->Enqueue([=](CGpuDeviceId device) {
        texture_ = cgpu_create_texture(device, &tex_desc);
        view_ = cgpu_create_texture_view(device, &tex_view_desc);
        upload_ready_fence_ = cgpu_create_fence(device);
        upload_ready_semaphore = cgpu_create_semaphore(device);
        resource_handle_ready_ = true;
    });
}

void RenderTexture::Destroy(struct RenderAuxThread* aux_thread)
{
    aux_thread->Enqueue([this](CGpuDeviceId device) {
        cgpu_free_texture(texture_);
        if (view_) cgpu_free_texture_view(view_);
        cgpu_free_fence(upload_ready_fence_);
        cgpu_free_semaphore(upload_ready_semaphore);
        texture_ = nullptr;
        view_ = nullptr;
        upload_ready_fence_ = nullptr;
        upload_ready_semaphore = nullptr;
    });
}

void RenderShader::Initialize(struct RenderAuxThread* aux_thread, const CGpuShaderLibraryDescriptor& desc)
{
    aux_thread->Enqueue([=](CGpuDeviceId device) {
        shader_ = cgpu_create_shader_library(device, &desc);
        resource_handle_ready_ = true;
    });
}

void RenderShader::Destroy(struct RenderAuxThread* aux_thread)
{
    aux_thread->Enqueue([this](CGpuDeviceId device) {
        cgpu_free_shader_library(shader_);
        shader_ = nullptr;
    });
}