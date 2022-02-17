#pragma once
#include "../common/utils.h"
#include "cgpu/cgpux.hpp"
#include "shaders.h"
#include <EASTL/vector_map.h>
#include <EASTL/unordered_set.h>
#include <EASTL/unordered_map.h>

struct PipelineKey {
    uint32_t vertex_layout_id_;
    CGpuRootSignatureId root_sig_;
    bool wireframe_mode_;
};

namespace eastl
{
template <typename T>
using cached_hashset = hash_set<T, eastl::hash<T>, eastl::equal_to<T>, EASTLAllocatorType, true>;

template <>
struct hash<PipelineKey> {
    size_t operator()(const PipelineKey& val) const { return skr_hash(&val, sizeof(PipelineKey), 0); }
};
template <>
struct equal_to<PipelineKey> {
    size_t operator()(const PipelineKey& a, const PipelineKey& b) const
    {
        const bool equal =
            (a.vertex_layout_id_ == b.vertex_layout_id_) &&
            (a.wireframe_mode_ == b.wireframe_mode_) &&
            (a.root_sig_ == b.root_sig_);
        return equal;
    }
};
} // namespace eastl

// D3D11-CreateDeviceAndSwapChain
class RenderDevice
{
public:
    void Initialize(ECGpuBackend backend);
    void Destroy();
    const eastl::cached_hashset<CGpuVertexLayout>* GetVertexLayouts() const;
    size_t AddVertexLayout(const CGpuVertexLayout& layout);

    const uint32_t* get_vertex_shader();
    const uint32_t get_vertex_shader_size();
    const uint32_t* get_fragment_shader();
    const uint32_t get_fragment_shader_size();
    FORCEINLINE CGpuDeviceId GetCGPUDevice() { return device_; }
    FORCEINLINE CGpuQueueId GetCGPUQueue() { return gfx_queue_; }
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
    CGpuRenderPipelineId CreateRenderPipeline(const PipelineKey& key);

protected:
    void freeRenderPipeline(CGpuRenderPipelineId pipeline);

    void asyncTransfer(const CGpuBufferToBufferTransfer* transfers, const ECGpuResourceState* dst_states,
        uint32_t transfer_count, CGpuSemaphoreId semaphore, CGpuFenceId fence = nullptr);

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
    // samplers
    CGpuSamplerId default_sampler_;
    // shaders & root_sigs
    CGpuShaderLibraryId vs_library_;
    CGpuShaderLibraryId fs_library_;
    CGpuPipelineShaderDescriptor ppl_shaders_[2];
    CGpuRootSignatureId root_sig_;
    // vertex layouts
    eastl::cached_hashset<CGpuVertexLayout> vertex_layouts_;
    // pipelines
    eastl::unordered_map<PipelineKey, CGpuRenderPipelineId> pipelines_;
};

FORCEINLINE const eastl::cached_hashset<CGpuVertexLayout>* RenderDevice::GetVertexLayouts() const
{
    return &vertex_layouts_;
}

FORCEINLINE size_t RenderDevice::AddVertexLayout(const CGpuVertexLayout& layout)
{
    const auto hash = vertex_layouts_.get_hash_code(layout);
    auto iter = vertex_layouts_.find_by_hash(hash);
    if (iter == vertex_layouts_.end())
    {
        vertex_layouts_.insert(layout);
        return hash;
    }
    return hash;
}

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
