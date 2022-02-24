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
    void Initialize(RenderDevice* render_device);
    void Destroy();

    // window
    SDL_Window* sdl_window_;
    SDL_SysWMinfo wmInfo;
    // surface and swapchain
    CGpuSurfaceId surface_;
    CGpuSwapChainId swapchain_;
    CGpuTextureViewId views_[3] = { nullptr, nullptr, nullptr };
    CGpuTextureId screen_ds_[3] = { nullptr, nullptr, nullptr };
    CGpuTextureViewId screen_ds_view_[3] = { nullptr, nullptr, nullptr };
};

// D3D11-CreateDeviceAndSwapChain
class RenderDevice
{
    friend class RenderWindow;
    friend struct RenderBlackboard;

public:
    void Initialize(ECGpuBackend backend, RenderWindow** render_window);
    void Destroy();

    const uint32_t* get_vertex_shader();
    const uint32_t get_vertex_shader_size();
    const uint32_t* get_fragment_shader();
    const uint32_t get_fragment_shader_size();
    FORCEINLINE CGpuDeviceId GetCGPUDevice() { return device_; }
    FORCEINLINE CGpuQueueId GetCGPUQueue() { return gfx_queue_; }
    FORCEINLINE ECGpuFormat GetScreenFormat() { return screen_format_; }
    FORCEINLINE CGpuRootSignatureId GetCGPUSignature() { return root_sig_; }

    template <typename Transfer>
    CGpuSemaphoreId AsyncTransfer(const Transfer* transfers, const ECGpuResourceState* dst_states,
        uint32_t transfer_count, CGpuFenceId fence = nullptr)
    {
        CGpuSemaphoreId semaphore = AllocSemaphore();
        asyncTransfer(transfers, dst_states, transfer_count, semaphore, fence);
        return semaphore;
    }
    CGpuSemaphoreId AllocSemaphore();
    void FreeSemaphore(CGpuSemaphoreId semaphore);
    CGpuFenceId AllocFence();
    void FreeFence(CGpuFenceId fence);
    CGpuDescriptorSetId CreateDescriptorSet(const CGpuRootSignatureId signature, uint32_t set_index);
    void FreeDescriptorSet(CGpuDescriptorSetId desc_set);
    uint32_t AcquireNextFrame(RenderWindow* window, const CGpuAcquireNextDescriptor& acquire);
    void Present(RenderWindow* window, uint32_t index, const CGpuSemaphoreId* wait_semaphores, uint32_t semaphore_count);
    void Submit(class RenderContext* context);
    void WaitIdle();
    void CollectGarbage(bool wait_idle = false);

protected:
    void freeRenderPipeline(CGpuRenderPipelineId pipeline);
    void asyncTransfer(const CGpuBufferToBufferTransfer* transfers, const ECGpuResourceState* dst_states,
        uint32_t transfer_count, CGpuSemaphoreId semaphore, CGpuFenceId fence = nullptr);

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
    eastl::unordered_map<CGpuFenceId, CGpuCommandBufferId> async_cpy_cmds_;
    // samplers
    CGpuSamplerId default_sampler_;
    // shaders & root_sigs
    CGpuShaderLibraryId vs_library_;
    CGpuShaderLibraryId fs_library_;
    CGpuPipelineShaderDescriptor ppl_shaders_[2];
    CGpuRootSignatureId root_sig_;
};

FORCEINLINE const uint32_t* RenderDevice::get_vertex_shader()
{
    if (backend_ == CGPU_BACKEND_VULKAN) return (const uint32_t*)vertex_shader_spirv;
    if (backend_ == CGPU_BACKEND_D3D12) return (const uint32_t*)vertex_shader_dxil;
    return CGPU_NULLPTR;
}
FORCEINLINE const uint32_t RenderDevice::get_vertex_shader_size()
{
    if (backend_ == CGPU_BACKEND_VULKAN) return sizeof(vertex_shader_spirv);
    if (backend_ == CGPU_BACKEND_D3D12) return sizeof(vertex_shader_dxil);
    return 0;
}
FORCEINLINE const uint32_t* RenderDevice::get_fragment_shader()
{
    if (backend_ == CGPU_BACKEND_VULKAN) return (const uint32_t*)fragment_shader_spirv;
    if (backend_ == CGPU_BACKEND_D3D12) return (const uint32_t*)fragment_shader_dxil;
    return CGPU_NULLPTR;
}
FORCEINLINE const uint32_t RenderDevice::get_fragment_shader_size()
{
    if (backend_ == CGPU_BACKEND_VULKAN) return sizeof(fragment_shader_spirv);
    if (backend_ == CGPU_BACKEND_D3D12) return sizeof(fragment_shader_dxil);
    return 0;
}
