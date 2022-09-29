#pragma once
#include "SkrRenderer/skr_renderer.configure.h"
#include "primitive_pass.h"
#include "effect_processor.h"

struct SKR_RENDERER_API SRenderer {
#ifdef __cplusplus
    virtual ~SRenderer() = default;
    virtual void render(skr::render_graph::RenderGraph* render_graph) = 0;

    virtual SRenderDeviceId get_render_device() const = 0;
    virtual dual_storage_t* get_dual_storage() const = 0;
#endif
};

#ifdef __cplusplus
    #include "module/module_manager.hpp"
    #include "render_device.h"

class SKR_RENDERER_API SkrRendererModule : public skr::IDynamicModule
{
public:
    virtual void on_load(int argc, char** argv) override;
    virtual void on_unload() override;

    SRenderDeviceId get_render_device();
#ifdef _WIN32
    skr_win_dstorage_decompress_service_id get_win_dstorage_decompress_service() const;
#endif
    static SkrRendererModule* Get();
protected:
    skr::RendererDevice render_device;
};
#endif

RUNTIME_EXTERN_C SKR_RENDERER_API 
SRendererId skr_create_renderer(SRenderDeviceId render_device, dual_storage_t* storage);

RUNTIME_EXTERN_C SKR_RENDERER_API 
void skr_free_renderer(SRendererId renderer);

RUNTIME_EXTERN_C SKR_RENDERER_API 
SRenderDeviceId skr_get_default_render_device();

RUNTIME_EXTERN_C SKR_RENDERER_API 
void skr_renderer_render_frame(SRendererId renderer, skr::render_graph::RenderGraph* render_graph);

#ifdef _WIN32
RUNTIME_EXTERN_C SKR_RENDERER_API
skr_win_dstorage_decompress_service_id skr_renderer_get_win_dstorage_decompress_service();
#endif