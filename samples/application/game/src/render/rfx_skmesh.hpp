#pragma once
#include "rfx_mesh.hpp"

static const skr_render_effect_name_t forward_effect_skin_name = u8"ForwardEffectSkin";
struct RenderEffectForwardSkin : public RenderEffectForward
{
    RenderEffectForwardSkin(skr_vfs_t* resource_vfs)
        : RenderEffectForward(resource_vfs) {}

    void on_register(SRendererId renderer, dual_storage_t* storage) override;
    void on_unregister(SRendererId renderer, dual_storage_t* storage) override;
    
    void initialize_queries(dual_storage_t* storage);
    void release_queries();

    dual_query_t* install_query = nullptr;
};