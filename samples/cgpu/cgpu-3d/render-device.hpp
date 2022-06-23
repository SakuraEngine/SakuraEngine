#pragma once
#include "../common/utils.h"
#include "cgpu/cgpux.hpp"
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
    CGPUSemaphoreId AcquireNextFrame(uint32_t& back_buffer_index);
    void BeginScreenPass(class RenderContext* ctx);
    void EndScreenPass(class RenderContext* ctx);
    void Present(uint32_t index, const CGPUSemaphoreId* wait_semaphores = nullptr, uint32_t semaphore_count = 0);

    SDL_Window* GetSDLWindow() { return sdl_window_; }

    static const auto SampleCount = CGPU_SAMPLE_COUNT_4;

protected:
    // window
    SDL_Window* sdl_window_;
    SDL_SysWMinfo wmInfo;
    // surface and swapchain
    CGPUSurfaceId surface_;
    CGPUSwapChainId swapchain_;
    CGPUTextureViewId views_[3] = { nullptr, nullptr, nullptr };

protected:
    CGPUTextureId screen_ds_ = nullptr;
    CGPUTextureViewId screen_ds_view_ = nullptr;
    CGPUTextureId msaa_render_targets_[3] = { nullptr, nullptr, nullptr };
    CGPUTextureViewId msaa_render_target_views_[3] = { nullptr, nullptr, nullptr };
    CGPUSemaphoreId present_semaphores_[3] = { nullptr, nullptr, nullptr };
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
    void Initialize(ECGPUBackend backend, class RenderWindow** render_window);
    void Destroy();

    FORCEINLINE CGPUDeviceId GetCGPUDevice() { return device_; }
    FORCEINLINE CGPUQueueId GetGraphicsQueue() { return gfx_queue_; }
    FORCEINLINE CGPUQueueId GetPresentQueue() { return gfx_queue_; }
    FORCEINLINE CGPUQueueId GetCopyQueue() { return cpy_queue_; }
    FORCEINLINE CGPUQueueId GetAsyncCopyQueue()
    {
        CGPUQueueId queue_ = cpy_queue_;
        if (extra_cpy_queue_cursor_ > 0)
        {
            queue_ = extra_cpy_queues_[extra_cpy_queue_cursor_.load() - 1];
        }
        extra_cpy_queue_cursor_++;
        extra_cpy_queue_cursor_ = extra_cpy_queue_cursor_.load() % (1 + extra_cpy_queue_count_);
        return queue_;
    }
    FORCEINLINE ECGPUFormat
    GetScreenFormat() { return screen_format_; }
    FORCEINLINE CGPURootSignatureId GetCGPUSignature() { return root_sig_; }
    FORCEINLINE bool AsyncCopyQueueEnabled() const { return gfx_queue_ != cpy_queue_; }

    CGPUSemaphoreId AllocSemaphore();
    void FreeSemaphore(CGPUSemaphoreId semaphore);
    CGPUFenceId AllocFence();
    void FreeFence(CGPUFenceId fence);
    CGPUDescriptorSetId CreateDescriptorSet(const CGPURootSignatureId signature, uint32_t set_index);
    void FreeDescriptorSet(CGPUDescriptorSetId desc_set);
    void Submit(class RenderContext* context);
    void WaitIdle();

protected:
    void freeRenderPipeline(CGPURenderPipelineId pipeline);

    ECGPUBackend backend_;
    ECGPUFormat screen_format_;
    // instance & adapter & device
    CGPUInstanceId instance_;
    CGPUAdapterId adapter_;
    CGPUDeviceId device_;
    CGPUQueueId gfx_queue_;
    // cpy
    CGPUCommandPoolId cpy_cmd_pool_;
    CGPUQueueId cpy_queue_;
    CGPUQueueId extra_cpy_queues_[7] = { nullptr };
    uint32_t extra_cpy_queue_count_ = 0;
    std::atomic_int32_t extra_cpy_queue_cursor_ = 0;
    // samplers
    CGPUSamplerId default_sampler_;
    // shaders & root_sigs
    CGPUShaderLibraryId vs_library_;
    CGPUShaderLibraryId fs_library_;
    CGPUPipelineShaderDescriptor ppl_shaders_[2];
    CGPURootSignatureId root_sig_;
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
    CGPUBufferId raw_src;
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
    CGPUBufferId raw_src = nullptr;
    AsyncRenderBuffer* src = nullptr;
    uint64_t src_offset;
};

struct AsyncTransferThread {
    friend class RenderDevice;

    void Initialize(class RenderDevice* render_device);
    void Destroy();

    template <typename Transfer>
    void AsyncTransfer(const Transfer* transfers, uint32_t transfer_count, CGPUFenceId fence = nullptr)
    {
        asyncTransfer(transfers, transfer_count, nullptr, fence);
    }
    AsyncRenderTexture* UploadTexture(AsyncRenderTexture* target, const void* data, size_t data_size, CGPUFenceId fence);

protected:
    void asyncTransfer(const AsyncBufferToBufferTransfer* transfers,
    uint32_t transfer_count, CGPUSemaphoreId semaphore, CGPUFenceId fence = nullptr);
    void asyncTransfer(const AsyncBufferToTextureTransfer* transfers,
    uint32_t transfer_count, CGPUSemaphoreId semaphore, CGPUFenceId fence = nullptr);
    RenderDevice* render_device_;
    CGPUCommandPoolId cpy_cmd_pool_;
    eastl::unordered_map<CGPUFenceId, CGPUCommandBufferId> async_cpy_cmds_;
    eastl::unordered_map<CGPUFenceId, CGPUBufferId> async_cpy_bufs_;
};
