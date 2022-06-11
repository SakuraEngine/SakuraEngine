#pragma once
#ifdef __cplusplus
#include "skr_renderer_config.h"
#include "module/module_manager.hpp"
#include "render_graph/frontend/render_graph.hpp"
#include "platform/window.h"

class SKR_RENDERER_API SkrRendererModule : public skr::IDynamicModule
{
public:
    virtual void on_load() override;
    virtual void on_unload() override;

    CGPUSwapChainId register_window(SWindowHandle window);

    CGPUDeviceId get_cgpu_device() const;
    CGPUQueueId get_gfx_queue() const;
    ECGPUFormat get_swapchain_format() const;
    CGPUSamplerId get_linear_sampler() const;

    static SkrRendererModule* Get();

protected:
    void create_api_objects();

    uint32_t backbuffer_index = 0;
    CGPUSurfaceId surface = nullptr;
    CGPUSwapChainId swapchain = nullptr;
    ECGPUBackend backend = CGPU_BACKEND_VULKAN;
    CGPUInstanceId instance = nullptr;
    CGPUAdapterId adapter = nullptr;
    CGPUDeviceId device = nullptr;
    CGPUQueueId gfx_queue = nullptr;
    CGPUSamplerId linear_sampler = nullptr;
    skr::render_graph::RenderGraph* renderGraph = nullptr;
};
#endif

RUNTIME_EXTERN_C SKR_RENDERER_API CGPUSwapChainId 
skr_renderer_register_window(SWindowHandle window);

RUNTIME_EXTERN_C SKR_RENDERER_API ECGPUFormat 
skr_renderer_get_swapchain_format();

RUNTIME_EXTERN_C SKR_RENDERER_API CGPUSamplerId 
skr_renderer_get_linear_sampler();

RUNTIME_EXTERN_C SKR_RENDERER_API CGPUQueueId 
skr_renderer_get_gfx_queue();

RUNTIME_EXTERN_C SKR_RENDERER_API CGPUDeviceId 
skr_renderer_get_cgpu_device();

