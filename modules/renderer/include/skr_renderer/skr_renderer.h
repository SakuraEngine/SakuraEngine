#pragma once
#include "SkrRenderer/skr_renderer.configure.h"
#include "primitive_pass.h"
#include "effect_processor.h"
#include "cgpu/io.h"

struct SKR_RENDERER_API ISkrRenderer {
#ifdef __cplusplus
    virtual ~ISkrRenderer() = default;
    virtual void initialize(bool enable_debug_layer, bool enable_gpu_based_validation, bool enable_set_name) = 0;
    virtual void render(skr::render_graph::RenderGraph* render_graph, dual_storage_t* storage) = 0;
    virtual void finalize() = 0;
    virtual CGPUDeviceId get_cgpu_device() const = 0;
    virtual CGPUQueueId get_gfx_queue() const = 0;
    virtual CGPUQueueId get_cpy_queue(uint32_t idx) const = 0;
    virtual CGPUDStorageQueueId get_file_dstorage_queue() const = 0;
    virtual CGPUDStorageQueueId get_memory_dstorage_queue() const = 0;
    virtual ECGPUFormat get_swapchain_format() const = 0;
    virtual CGPUSamplerId get_linear_sampler() const = 0;
#endif
};

#ifdef __cplusplus
    #include "module/module_manager.hpp"
    #include "platform/window.h"
    #include "EASTL/vector_map.h"
#ifdef _WIN32
    #include "cgpu/extensions/dstorage_windows.h"
#endif
#include "cgpu/extensions/cgpu_nsight.h"

class SkrRendererModule;

namespace skr
{
struct SKR_RENDERER_API Renderer : public ISkrRenderer {
    friend class ::SkrRendererModule;

public:
    virtual ~Renderer() = default;
    virtual void initialize(bool enable_debug_layer, bool enable_gpu_based_validation, bool enable_set_name) override;
    virtual void render(skr::render_graph::RenderGraph* render_graph, dual_storage_t* storage) = 0;
    virtual void finalize() override;

    virtual CGPUDeviceId get_cgpu_device() const override
    {
        return device;
    }
    virtual CGPUQueueId get_gfx_queue() const override
    {
        return gfx_queue;
    }
    virtual CGPUQueueId get_cpy_queue(uint32_t idx) const override
    {
        if (idx < cpy_queues.size())
            return cpy_queues[idx];
        return cpy_queues[0];
    }
    virtual CGPUDStorageQueueId get_file_dstorage_queue() const override
    {
        return file_dstorage_queue;
    }
    virtual CGPUDStorageQueueId get_memory_dstorage_queue() const override
    {
        return memory_dstorage_queue;
    }
    virtual ECGPUFormat get_swapchain_format() const override
    {
        if (swapchains.size())
            return (ECGPUFormat)swapchains.at(0).second->back_buffers[0]->format;
        return CGPU_FORMAT_B8G8R8A8_UNORM;
    }
    virtual CGPUSamplerId get_linear_sampler() const override
    {
        return linear_sampler;
    }

    CGPUSwapChainId register_window(SWindowHandle window);
    CGPUSwapChainId recreate_window_swapchain(SWindowHandle window);

protected:
    void create_api_objects(bool enable_debug_layer, bool enable_gpu_based_validation, bool enable_set_name);

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

class SKR_RENDERER_API SkrRendererModule : public skr::IDynamicModule
{
public:
    virtual void on_load(int argc, char** argv) override;
    virtual void on_unload() override;

    CGPUDeviceId get_cgpu_device() const;
    skr_io_vram_service_t* get_vram_service() const;
    CGPUQueueId get_gfx_queue() const;
    CGPUQueueId get_cpy_queue(uint32_t idx = 0) const;
    CGPUDStorageQueueId get_file_dstorage_queue() const;
    CGPUDStorageQueueId get_memory_dstorage_queue() const;
    ECGPUFormat get_swapchain_format() const;
    CGPUSamplerId get_linear_sampler() const;
    CGPURootSignaturePoolId get_root_signature_pool() const;
#ifdef _WIN32
    skr_win_dstorage_decompress_service_id get_win_dstorage_decompress_service() const;
#endif

    static SkrRendererModule* Get();
    skr::Renderer* get_renderer() { return renderer; }

protected:
    // Renderer
    skr::Renderer* renderer = nullptr;
};
#endif

RUNTIME_EXTERN_C SKR_RENDERER_API struct ISkrRenderer*
skr_renderer_get_renderer();

RUNTIME_EXTERN_C SKR_RENDERER_API CGPUSwapChainId
skr_renderer_register_window(SWindowHandle window);

RUNTIME_EXTERN_C SKR_RENDERER_API CGPUSwapChainId
skr_renderer_recreate_window_swapchain(SWindowHandle window);

RUNTIME_EXTERN_C SKR_RENDERER_API ECGPUFormat
skr_renderer_get_swapchain_format();

RUNTIME_EXTERN_C SKR_RENDERER_API CGPUSamplerId
skr_renderer_get_linear_sampler();

RUNTIME_EXTERN_C SKR_RENDERER_API CGPURootSignaturePoolId
skr_renderer_get_root_signature_pool();

RUNTIME_EXTERN_C SKR_RENDERER_API CGPUQueueId
skr_renderer_get_gfx_queue();

RUNTIME_EXTERN_C SKR_RENDERER_API CGPUDStorageQueueId
skr_renderer_get_file_dstorage_queue();

RUNTIME_EXTERN_C SKR_RENDERER_API CGPUDStorageQueueId
skr_renderer_get_memory_dstorage_queue();

RUNTIME_EXTERN_C SKR_RENDERER_API CGPUQueueId
skr_renderer_get_cpy_queue();

RUNTIME_EXTERN_C SKR_RENDERER_API CGPUQueueId
skr_renderer_get_nth_cpy_queue(uint32_t n);

RUNTIME_EXTERN_C SKR_RENDERER_API CGPUDeviceId
skr_renderer_get_cgpu_device();

RUNTIME_EXTERN_C SKR_RENDERER_API skr_io_vram_service_t*
skr_renderer_get_vram_service();

RUNTIME_EXTERN_C SKR_RENDERER_API void
skr_renderer_render_frame(skr::render_graph::RenderGraph* render_graph, dual_storage_t* storage);

#ifdef _WIN32
RUNTIME_EXTERN_C SKR_RENDERER_API
skr_win_dstorage_decompress_service_id skr_renderer_get_win_dstorage_decompress_service();
#endif