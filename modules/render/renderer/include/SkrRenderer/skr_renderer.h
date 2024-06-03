#pragma once
#include "SkrBase/config.h"
#include "SkrRenderer/primitive_pass.h"
#include "SkrRenderer/fwd_types.h"

struct sugoi_storage_t;
struct SViewportManager;

struct SKR_RENDERER_API SRenderer {
#ifdef __cplusplus
    virtual ~SRenderer() = default;
    virtual void render(skr::render_graph::RenderGraph* render_graph) = 0;

    virtual SRenderDeviceId get_render_device() const = 0;
    virtual sugoi_storage_t* get_sugoi_storage() const = 0;
    virtual SViewportManager* get_viewport_manager() const = 0;
#endif
};

#ifdef __cplusplus
    #include "SkrCore/module/module.hpp"
    #include "render_device.h"

class SKR_RENDERER_API SkrRendererModule : public skr::IDynamicModule
{
public:
    virtual void on_load(int argc, char8_t** argv) override;
    virtual void on_unload() override;

    SRenderDeviceId get_render_device();

    static SkrRendererModule* Get();
protected:
    skr::RendererDevice* render_device;
};
#endif

SKR_EXTERN_C SKR_RENDERER_API 
SRendererId skr_create_renderer(SRenderDeviceId render_device, sugoi_storage_t* storage);

SKR_EXTERN_C SKR_RENDERER_API 
void skr_free_renderer(SRendererId renderer);

SKR_EXTERN_C SKR_RENDERER_API 
SRenderDeviceId skr_get_default_render_device();

SKR_EXTERN_C SKR_RENDERER_API 
void skr_renderer_render_frame(SRendererId renderer, skr::render_graph::RenderGraph* render_graph);