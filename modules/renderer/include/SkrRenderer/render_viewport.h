#pragma once
#include "SkrBase/config.h"
#include "SkrScene/scene.h"
#ifndef __meta__
    #include "SkrRenderer/render_viewport.generated.h" // IWYU pragma: export
#endif

sreflect_struct("guid": "96fd4826-cb03-4286-8d14-8a86c9f96ee4", "component" : true)
skr_render_viewport_t
{
    // index registered in renderer
    uint32_t index;
    // derived from camera
    uint32_t viewport_width;
    // derived from camera
    uint32_t viewport_height;

    skr_float4x4_t view_projection;
};
typedef struct skr_render_viewport_t skr_render_viewport_t;

struct SKR_RENDERER_API SViewportManager {
#ifdef __cplusplus
    virtual uint32_t register_viewport(const char8_t* viewport_name) SKR_NOEXCEPT = 0;
    virtual skr_render_viewport_t* find_viewport(const char8_t* viewport_name) SKR_NOEXCEPT = 0;
    virtual skr_render_viewport_t* find_viewport(uint32_t idx) SKR_NOEXCEPT = 0;
    virtual void remove_viewport(const char8_t* viewport_name) SKR_NOEXCEPT = 0;
    virtual void remove_viewport(uint32_t idx) SKR_NOEXCEPT = 0;

    static SViewportManager* Create(sugoi_storage_t* storage);
    static void Free(SViewportManager* viewport_manager);
    virtual ~SViewportManager() SKR_NOEXCEPT;
#endif
};

SKR_RENDERER_EXTERN_C SKR_RENDERER_API
void skr_resolve_camera_to_viewport(const skr_camera_comp_t* camera, const skr_translation_comp_t* translation, skr_render_viewport_t* viewport);

SKR_RENDERER_EXTERN_C SKR_RENDERER_API
void skr_resolve_cameras_to_viewport(struct SViewportManager* viewport_manager, sugoi_storage_t* storage);

