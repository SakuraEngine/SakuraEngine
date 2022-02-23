#pragma once
#include "cgpu/cgpux.hpp"
#include "platform/thread.h"
#include <EASTL/vector.h>
#include <atomic>

// AsyncI/O & AsyncUpload
// 1.1 CreateRenderMemeoryObject  |  (Then handles are available for drawcalls)  Aux Thread
// 1.2 LoadDiskObject             |                                              I/O Thread
//                              2.1 AsyncTransfer                                Main/Aux Thread
//                              2.2 Acquire/Release Barrier                      Main/Aux Thread
struct RenderResource {
    inline virtual void Wait()
    {
        while (!resource_handle_ready_) {}
    }
    std::atomic_bool resource_handle_ready_;
};

struct RenderMemoryResource : public RenderResource {
    CGpuFenceId upload_ready_fence_;
    CGpuSemaphoreId upload_ready_semaphore;
};

struct RenderBuffer : public RenderMemoryResource {
    void Initialize(struct RenderAuxThread* aux_thread, const CGpuBufferDescriptor& buffer_desc);
    void Destroy(struct RenderAuxThread* aux_thread);

    CGpuBufferId buffer_;
};

struct RenderTexture : public RenderMemoryResource {
    void Initialize(struct RenderAuxThread* aux_thread, const CGpuTextureDescriptor& tex_desc, bool default_srv = true);
    void Initialize(struct RenderAuxThread* aux_thread, const CGpuTextureDescriptor& tex_desc, const CGpuTextureViewDescriptor& tex_view_desc);
    void Destroy(struct RenderAuxThread* aux_thread);

    CGpuTextureId texture_;
    CGpuTextureViewId view_;
};

struct RenderShader : public RenderResource {
    void Initialize(struct RenderAuxThread* aux_thread, const CGpuShaderLibraryDescriptor& desc);
    void Destroy(struct RenderAuxThread* aux_thread);

    CGpuShaderLibraryId shader_;
};

using AuxThreadTask = eastl::function<void(CGpuDeviceId)>;
struct RenderAuxThread {
    void Initialize(class RenderDevice* render_device);
    void Destroy();

    void Enqueue(const AuxThreadTask& task);
    void Wait();
    SThreadDesc aux_item_;
    SThreadHandle aux_thread_;
    SMutex load_mutex_;
    CGpuDeviceId device_;
    eastl::vector<AuxThreadTask> task_queue_;
    std::atomic_bool is_running_;
};