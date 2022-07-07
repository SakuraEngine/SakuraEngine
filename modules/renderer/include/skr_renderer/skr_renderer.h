#pragma once
#if !defined(__meta__)
    #include "SkrRenderer/skr_renderer.configure.h"
#endif
#ifdef __cplusplus
    #include "module/module_manager.hpp"
    #include "render_graph/frontend/render_graph.hpp"
    #include "platform/window.h"
    #include "EASTL/vector_map.h"

class SkrRendererModule;

namespace skr
{
struct SKR_RENDERER_API Renderer
{
    friend class ::SkrRendererModule;

public:
    virtual ~Renderer() = default;
    void initialize();
    void finalize();
    CGPUSwapChainId register_window(SWindowHandle window);

protected:
    void create_api_objects();

    struct dual_storage_t* storage;
    // Device objects
    uint32_t backbuffer_index = 0;
    eastl::vector_map<SWindowHandle, CGPUSurfaceId> surfaces;
    eastl::vector_map<SWindowHandle, CGPUSwapChainId> swapchains;
    ECGPUBackend backend = CGPU_BACKEND_VULKAN;
    CGPUInstanceId instance = nullptr;
    CGPUAdapterId adapter = nullptr;
    CGPUDeviceId device = nullptr;
    CGPUQueueId gfx_queue = nullptr;
    CGPUSamplerId linear_sampler = nullptr;
    // Render graph
    skr::render_graph::RenderGraph* renderGraph = nullptr;
};
} // namespace skr

class SKR_RENDERER_API SkrRendererModule : public skr::IDynamicModule
{
public:
    virtual void on_load(int argc, char** argv) override;
    virtual void on_unload() override;

    CGPUDeviceId get_cgpu_device() const;
    CGPUQueueId get_gfx_queue() const;
    ECGPUFormat get_swapchain_format() const;
    CGPUSamplerId get_linear_sampler() const;

    static SkrRendererModule* Get();
    skr::Renderer* get_renderer() {return &renderer;}

protected:
    // Renderer
    skr::Renderer renderer;
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
