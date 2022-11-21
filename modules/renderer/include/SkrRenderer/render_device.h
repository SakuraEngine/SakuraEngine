#pragma once
#include "SkrRenderer/module.configure.h"
#include "fwd_types.h"
#include "cgpu/io.h"
#ifdef _WIN32
#include "cgpu/extensions/dstorage_windows.h"
#endif

struct skr_threaded_service_t;

#ifdef __cplusplus
#include "platform/window.h"

namespace skr
{
struct SKR_RENDERER_API RendererDevice
{
    struct Builder
    {
        ECGPUBackend backend;
        bool enable_debug_layer = false;
        bool enable_gpu_based_validation = false;
        bool enable_set_name = true;
        uint32_t aux_thread_count = 0;
    };
    static RendererDevice* Create() SKR_NOEXCEPT;
    static void Free(RendererDevice* device) SKR_NOEXCEPT;
    
    virtual void initialize(const Builder& builder) = 0;
    virtual void finalize() = 0;

    virtual CGPUSwapChainId register_window(SWindowHandle window) = 0;
    virtual CGPUSwapChainId recreate_window_swapchain(SWindowHandle window) = 0;
    virtual void create_api_objects(const Builder& builder) = 0;

    virtual CGPUDeviceId get_cgpu_device() const = 0;
    virtual ECGPUBackend get_backend() const = 0;
    virtual CGPUQueueId get_gfx_queue() const = 0;
    virtual CGPUQueueId get_cpy_queue(uint32_t idx = 0) const = 0;
    virtual CGPUDStorageQueueId get_file_dstorage_queue() const = 0;
    virtual CGPUDStorageQueueId get_memory_dstorage_queue() const = 0;
    virtual ECGPUFormat get_swapchain_format() const = 0;
    virtual CGPUSamplerId get_linear_sampler() const = 0;
    virtual CGPURootSignaturePoolId get_root_signature_pool() const = 0;
    virtual skr_io_vram_service_t* get_vram_service() const = 0;

    virtual uint32_t get_aux_service_count() const = 0;
    virtual skr_threaded_service_t* get_aux_service(uint32_t index) const = 0;

#ifdef _WIN32
    virtual skr_win_dstorage_decompress_service_id get_win_dstorage_decompress_service() const = 0;
#endif
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

#ifdef _WIN32
RUNTIME_EXTERN_C SKR_RENDERER_API
skr_win_dstorage_decompress_service_id skr_render_device_get_win_dstorage_decompress_service(SRenderDeviceId device);
#endif