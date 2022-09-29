#pragma once
#include "SkrRenderer/skr_renderer.configure.h"
#include "primitive_pass.h"
#include "effect_processor.h"
#include "cgpu/io.h"

#ifdef __cplusplus
#include "platform/window.h"
#include "EASTL/vector_map.h"
#ifdef _WIN32
    #include "cgpu/extensions/dstorage_windows.h"
#endif
#include "cgpu/extensions/cgpu_nsight.h"

namespace skr
{
struct SKR_RENDERER_API RendererDevice
{
    void initialize(bool enable_debug_layer, bool enable_gpu_based_validation, bool enable_set_name);
    void finalize();

    CGPUSwapChainId register_window(SWindowHandle window);
    CGPUSwapChainId recreate_window_swapchain(SWindowHandle window);
    void create_api_objects(bool enable_debug_layer, bool enable_gpu_based_validation, bool enable_set_name);

    CGPUDeviceId get_cgpu_device() const
    {
        return device;
    }

    CGPUQueueId get_gfx_queue() const
    {
        return gfx_queue;
    }

    CGPUQueueId get_cpy_queue(uint32_t idx = 0) const
    {
        if (idx < cpy_queues.size())
            return cpy_queues[idx];
        return cpy_queues[0];
    }

    CGPUDStorageQueueId get_file_dstorage_queue() const
    {
        return file_dstorage_queue;
    }

    CGPUDStorageQueueId get_memory_dstorage_queue() const
    {
        return memory_dstorage_queue;
    }

    ECGPUFormat get_swapchain_format() const
    {
        if (swapchains.size())
            return (ECGPUFormat)swapchains.at(0).second->back_buffers[0]->format;
        return CGPU_FORMAT_B8G8R8A8_UNORM;
    }

    CGPUSamplerId get_linear_sampler() const
    {
        return linear_sampler;
    }

    CGPURootSignaturePoolId get_root_signature_pool() const
    {
        return root_signature_pool;
    }

    skr_io_vram_service_t* get_vram_service() const
    {
        return vram_service;
    }

    // Device objects
    uint32_t backbuffer_index = 0;
    eastl::vector_map<SWindowHandle, CGPUSurfaceId> surfaces;
    eastl::vector_map<SWindowHandle, CGPUSwapChainId> swapchains;
    ECGPUBackend backend = CGPU_BACKEND_VULKAN;
    CGPUInstanceId instance = nullptr;
    CGPUAdapterId adapter = nullptr;
    CGPUDeviceId device = nullptr;
    CGPUQueueId gfx_queue = nullptr;
    eastl::vector<CGPUQueueId> cpy_queues;
    CGPUSamplerId linear_sampler = nullptr;
    skr_io_vram_service_t* vram_service = nullptr;
    CGPUDStorageQueueId file_dstorage_queue = nullptr;
    CGPUDStorageQueueId memory_dstorage_queue = nullptr;
    CGPURootSignaturePoolId root_signature_pool = nullptr;
#ifdef _WIN32
    skr_win_dstorage_decompress_service_id decompress_service = nullptr;
#endif
    CGPUNSightTrackerId nsight_tracker;
};
} // namespace skr
#endif

RUNTIME_EXTERN_C SKR_RENDERER_API 
CGPUSwapChainId skr_render_device_register_window(SRenderDeviceId device, SWindowHandle window);

RUNTIME_EXTERN_C SKR_RENDERER_API 
CGPUSwapChainId skr_render_device_recreate_window_swapchain(SRenderDeviceId device, SWindowHandle window);

RUNTIME_EXTERN_C SKR_RENDERER_API 
ECGPUFormat skr_render_device_get_swapchain_format(SRenderDeviceId device);

RUNTIME_EXTERN_C SKR_RENDERER_API 
CGPUSamplerId skr_render_device_get_linear_sampler(SRenderDeviceId device);

RUNTIME_EXTERN_C SKR_RENDERER_API 
CGPURootSignaturePoolId skr_render_device_get_root_signature_pool(SRenderDeviceId device);

RUNTIME_EXTERN_C SKR_RENDERER_API 
CGPUQueueId skr_render_device_get_gfx_queue(SRenderDeviceId device);

RUNTIME_EXTERN_C SKR_RENDERER_API 
CGPUDStorageQueueId skr_render_device_get_file_dstorage_queue(SRenderDeviceId device);

RUNTIME_EXTERN_C SKR_RENDERER_API 
CGPUDStorageQueueId skr_render_device_get_memory_dstorage_queue(SRenderDeviceId device);

RUNTIME_EXTERN_C SKR_RENDERER_API 
CGPUQueueId skr_render_device_get_cpy_queue(SRenderDeviceId device);

RUNTIME_EXTERN_C SKR_RENDERER_API 
CGPUQueueId skr_render_device_get_nth_cpy_queue(SRenderDeviceId device, uint32_t n);

RUNTIME_EXTERN_C SKR_RENDERER_API 
CGPUDeviceId skr_render_device_get_cgpu_device(SRenderDeviceId device);

RUNTIME_EXTERN_C SKR_RENDERER_API 
skr_io_vram_service_t* skr_render_device_get_vram_service(SRenderDeviceId device);