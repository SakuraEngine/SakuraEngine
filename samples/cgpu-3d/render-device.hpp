#pragma once
#include "../common/utils.h"
#include "cgpu/cgpux.hpp"
#include "shaders.h"
#include "render-resources.hpp"
#include <EASTL/vector_map.h>
#include <EASTL/unordered_set.h>
#include <EASTL/string.h>

class RenderWindow
{
    friend class RenderDevice;

public:
    RenderWindow() = default;
    void Initialize(class RenderDevice* render_device);
    void Destroy();
    CGpuSemaphoreId AcquireNextFrame(uint32_t& back_buffer_index);
    void BeginScreenPass(class RenderContext* ctx);
    void EndScreenPass(class RenderContext* ctx);
    void Present(uint32_t index, const CGpuSemaphoreId* wait_semaphores = nullptr, uint32_t semaphore_count = 0);
    static const auto SampleCount = SAMPLE_COUNT_8;

    // window
    SDL_Window* sdl_window_;
    SDL_SysWMinfo wmInfo;
    // surface and swapchain
    CGpuSurfaceId surface_;
    CGpuSwapChainId swapchain_;
    CGpuTextureViewId views_[3] = { nullptr, nullptr, nullptr };

protected:
    CGpuTextureId screen_ds_[3] = { nullptr, nullptr, nullptr };
    CGpuTextureViewId screen_ds_view_[3] = { nullptr, nullptr, nullptr };
    CGpuTextureId msaa_render_targets_[3] = { nullptr, nullptr, nullptr };
    CGpuTextureViewId msaa_render_target_views_[3] = { nullptr, nullptr, nullptr };
    CGpuSemaphoreId present_semaphores_[3] = { nullptr, nullptr, nullptr };
    uint32_t present_semaphores_cursor_ = 0;
    uint32_t backbuffer_index_ = 0;
    RenderDevice* render_device_ = nullptr;
};

// D3D11-CreateDeviceAndSwapChain
class RenderDevice
{
    friend class RenderWindow;
    friend struct RenderBlackboard;
    friend struct RenderAuxThread;
    friend struct AsyncTransferThread;

public:
    void Initialize(ECGpuBackend backend, class RenderWindow** render_window);
    void Destroy();

    FORCEINLINE CGpuDeviceId GetCGPUDevice() { return device_; }
    FORCEINLINE CGpuQueueId GetGraphicsQueue() { return gfx_queue_; }
    FORCEINLINE CGpuQueueId GetPresentQueue() { return gfx_queue_; }
    FORCEINLINE CGpuQueueId GetCopyQueue() { return cpy_queue_; }
    FORCEINLINE CGpuQueueId GetAsyncCopyQueue()
    {
        CGpuQueueId queue_ = cpy_queue_;
        if (extra_cpy_queue_cursor_ > 0)
        {
            queue_ = extra_cpy_queues_[extra_cpy_queue_cursor_.load() - 1];
        }
        extra_cpy_queue_cursor_++;
        extra_cpy_queue_cursor_ = extra_cpy_queue_cursor_.load() % (1 + extra_cpy_queue_count_);
        return queue_;
    }
    FORCEINLINE ECGpuFormat
    GetScreenFormat() { return screen_format_; }
    FORCEINLINE CGpuRootSignatureId GetCGPUSignature() { return root_sig_; }
    FORCEINLINE bool AsyncCopyQueueEnabled() const { return gfx_queue_ != cpy_queue_; }

    CGpuSemaphoreId AllocSemaphore();
    void FreeSemaphore(CGpuSemaphoreId semaphore);
    CGpuFenceId AllocFence();
    void FreeFence(CGpuFenceId fence);
    CGpuDescriptorSetId CreateDescriptorSet(const CGpuRootSignatureId signature, uint32_t set_index);
    void FreeDescriptorSet(CGpuDescriptorSetId desc_set);
    void Submit(class RenderContext* context);
    void WaitIdle();

protected:
    const uint32_t* getVertexShader();
    const uint32_t getVertexShaderSize();
    const uint32_t* getFragmentShader();
    const uint32_t getFragmentShaderSize();
    void freeRenderPipeline(CGpuRenderPipelineId pipeline);

    ECGpuBackend backend_;
    ECGpuFormat screen_format_;
    // instance & adapter & device
    CGpuInstanceId instance_;
    CGpuAdapterId adapter_;
    CGpuDeviceId device_;
    CGpuQueueId gfx_queue_;
    // cpy
    CGpuCommandPoolId cpy_cmd_pool_;
    CGpuQueueId cpy_queue_;
    CGpuQueueId extra_cpy_queues_[7] = { nullptr };
    uint32_t extra_cpy_queue_count_ = 0;
    std::atomic_int32_t extra_cpy_queue_cursor_ = 0;
    // samplers
    CGpuSamplerId default_sampler_;
    // shaders & root_sigs
    CGpuShaderLibraryId vs_library_;
    CGpuShaderLibraryId fs_library_;
    CGpuPipelineShaderDescriptor ppl_shaders_[2];
    CGpuRootSignatureId root_sig_;
};

struct RenderAuxThread {
    friend class RenderDevice;

    virtual ~RenderAuxThread() = default;
    virtual void Initialize(class RenderDevice* render_device);
    virtual void Destroy();
    virtual void Wait();

    void Enqueue(const AuxThreadTaskWithCallback& task);
    SThreadDesc aux_item_;
    SThreadHandle aux_thread_;
    SMutex load_mutex_;
    RenderDevice* render_device_;
    eastl::vector<AuxThreadTaskWithCallback> task_queue_;
    std::atomic_bool is_running_;
    bool force_block_ = false;
};

struct AsyncBufferToBufferTransfer {
    AsyncRenderBuffer* dst;
    uint64_t dst_offset;
    CGpuBufferId raw_src;
    AsyncRenderBuffer* src;
    uint64_t src_offset;
    uint64_t size;
};

struct AsyncBufferToTextureTransfer {
    AsyncRenderTexture* dst;
    uint32_t dst_mip_level;
    uint32_t elems_per_row;
    uint32_t rows_per_image;
    uint32_t base_array_layer;
    uint32_t layer_count;
    CGpuBufferId raw_src = nullptr;
    AsyncRenderBuffer* src = nullptr;
    uint64_t src_offset;
};

struct AsyncTransferThread {
    friend class RenderDevice;

    void Initialize(class RenderDevice* render_device);
    void Destroy();

    template <typename Transfer>
    void AsyncTransfer(const Transfer* transfers, uint32_t transfer_count, CGpuFenceId fence = nullptr)
    {
        asyncTransfer(transfers, transfer_count, nullptr, fence);
    }
    AsyncRenderTexture* UploadTexture(AsyncRenderTexture* target, const void* data, size_t data_size, CGpuFenceId fence);

protected:
    void asyncTransfer(const AsyncBufferToBufferTransfer* transfers,
        uint32_t transfer_count, CGpuSemaphoreId semaphore, CGpuFenceId fence = nullptr);
    void asyncTransfer(const AsyncBufferToTextureTransfer* transfers,
        uint32_t transfer_count, CGpuSemaphoreId semaphore, CGpuFenceId fence = nullptr);
    RenderDevice* render_device_;
    CGpuCommandPoolId cpy_cmd_pool_;
    eastl::unordered_map<CGpuFenceId, CGpuCommandBufferId> async_cpy_cmds_;
    eastl::unordered_map<CGpuFenceId, CGpuBufferId> async_cpy_bufs_;
};

FORCEINLINE const uint32_t* RenderDevice::getVertexShader()
{
    if (backend_ == CGPU_BACKEND_VULKAN) return (const uint32_t*)vertex_shader_spirv;
    if (backend_ == CGPU_BACKEND_D3D12) return (const uint32_t*)vertex_shader_dxil;
    return CGPU_NULLPTR;
}
FORCEINLINE const uint32_t RenderDevice::getVertexShaderSize()
{
    if (backend_ == CGPU_BACKEND_VULKAN) return sizeof(vertex_shader_spirv);
    if (backend_ == CGPU_BACKEND_D3D12) return sizeof(vertex_shader_dxil);
    return 0;
}
FORCEINLINE const uint32_t* RenderDevice::getFragmentShader()
{
    if (backend_ == CGPU_BACKEND_VULKAN) return (const uint32_t*)fragment_shader_spirv;
    if (backend_ == CGPU_BACKEND_D3D12) return (const uint32_t*)fragment_shader_dxil;
    return CGPU_NULLPTR;
}
FORCEINLINE const uint32_t RenderDevice::getFragmentShaderSize()
{
    if (backend_ == CGPU_BACKEND_VULKAN) return sizeof(fragment_shader_spirv);
    if (backend_ == CGPU_BACKEND_D3D12) return sizeof(fragment_shader_dxil);
    return 0;
}
