#pragma once
#include "../common/utils.h"
#include "cgpu/cgpux.hpp"
#include "shaders.h"
#include <EASTL/vector_map.h>
#include <EASTL/unordered_set.h>
#include <EASTL/unordered_map.h>

// D3D11-CreateDeviceAndSwapChain
class RenderDevice
{
public:
    void Initialize(ECGpuBackend backend);
    void Destroy();
    FORCEINLINE const eastl::unordered_set<CGpuVertexLayout>* GetVertexLayouts() const
    {
        return &vertex_layouts_;
    }
    FORCEINLINE void AddVertexLayout(const CGpuVertexLayout& layout)
    {
        auto iter = vertex_layouts_.find(layout);
        if (iter == vertex_layouts_.end())
        {
            vertex_layouts_.insert(layout);
        }
    }
    FORCEINLINE CGpuDeviceId GetCGPUDevice() { return device_; }
    FORCEINLINE CGpuQueueId GetCGPUQueue() { return gfx_queue_; }
    FORCEINLINE const uint32_t* get_vertex_shader()
    {
        if (backend_ == CGPU_BACKEND_VULKAN) return (const uint32_t*)vertex_shader_spirv;
        if (backend_ == CGPU_BACKEND_D3D12) return (const uint32_t*)vertex_shader_dxil;
        return CGPU_NULLPTR;
    }
    FORCEINLINE const uint32_t get_vertex_shader_size()
    {
        if (backend_ == CGPU_BACKEND_VULKAN) return sizeof(vertex_shader_spirv);
        if (backend_ == CGPU_BACKEND_D3D12) return sizeof(vertex_shader_dxil);
        return 0;
    }
    FORCEINLINE const uint32_t* get_fragment_shader()
    {
        if (backend_ == CGPU_BACKEND_VULKAN) return (const uint32_t*)fragment_shader_spirv;
        if (backend_ == CGPU_BACKEND_D3D12) return (const uint32_t*)fragment_shader_dxil;
        return CGPU_NULLPTR;
    }
    FORCEINLINE const uint32_t get_fragment_shader_size()
    {
        if (backend_ == CGPU_BACKEND_VULKAN) return sizeof(fragment_shader_spirv);
        if (backend_ == CGPU_BACKEND_D3D12) return sizeof(fragment_shader_dxil);
        return 0;
    }
    template <typename Transfer>
    CGpuSemaphoreId AsyncTransfer(const Transfer* transfers, uint32_t transfer_count, CGpuFenceId fence = nullptr)
    {
        CGpuSemaphoreId semaphore = allocSemaphore();
        asyncTransfer(transfers, transfer_count, semaphore, fence);
        return semaphore;
    }
    void FreeSemaphore(CGpuSemaphoreId semaphore);
    CGpuFenceId AllocFence();
    void FreeFence(CGpuFenceId fence);

protected:
    CGpuSemaphoreId allocSemaphore();
    void asyncTransfer(const CGpuBufferToBufferTransfer* transfers, uint32_t transfer_count, CGpuSemaphoreId semaphore, CGpuFenceId fence = nullptr);

    struct PipelineKey {
        CGpuVertexLayout vertex_layout_;
    };
    ECGpuBackend backend_;
    // window
    SDL_Window* sdl_window_;
    SDL_SysWMinfo wmInfo;
    // instance & adapter & device
    CGpuInstanceId instance_;
    CGpuAdapterId adapter_;
    CGpuDeviceId device_;
    CGpuQueueId gfx_queue_;
    // cpy
    CGpuCommandPoolId cpy_cmd_pool_;
    CGpuQueueId cpy_queue_;
    eastl::vector_map<CGpuSemaphoreId, CGpuCommandBufferId> cpy_cmds;
    // surface and swapchain
    CGpuSurfaceId surface_;
    CGpuSwapChainId swapchain_;
    // shaders
    CGpuShaderLibraryId vs_library_;
    CGpuShaderLibraryId fs_library_;
    // vertex layouts
    eastl::unordered_set<CGpuVertexLayout> vertex_layouts_;
    // pipelines
    eastl::unordered_map<PipelineKey, CGpuRenderPipelineId> pipelines_;
};
