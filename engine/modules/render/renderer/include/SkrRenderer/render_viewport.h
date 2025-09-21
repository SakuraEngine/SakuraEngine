#pragma once
#include "SkrBase/config.h"
#include "SkrSceneCore/transform_system.h"
#include "SkrRenderer/render_viewport.generated.h" // IWYU pragma: export

struct [[sattr(
    guid = "96fd4826-cb03-4286-8d14-8a86c9f96ee4";
    ecs.comp = @enable;
)]] skr_render_viewport_t
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

struct SKR_RENDERER_API SViewportManager
{
#ifdef __cplusplus
    virtual uint32_t register_viewport(const char8_t* viewport_name) SKR_NOEXCEPT = 0;
    virtual skr_render_viewport_t* find_viewport(const char8_t* viewport_name) SKR_NOEXCEPT = 0;
    virtual skr_render_viewport_t* find_viewport(uint32_t idx) SKR_NOEXCEPT = 0;
    virtual void remove_viewport(const char8_t* viewport_name) SKR_NOEXCEPT = 0;
    virtual void remove_viewport(uint32_t idx) SKR_NOEXCEPT = 0;

    static SViewportManager* Create(skr::ecs::ECSWorld* storage);
    static void Destroy(SViewportManager* viewport_manager);
    virtual ~SViewportManager() SKR_NOEXCEPT;
#endif
};

SKR_EXTERN_C SKR_RENDERER_API void skr_resolve_camera_to_viewport(const skr::scene::CameraComponent* camera, const skr::scene::PositionComponent* translation, skr_render_viewport_t* viewport);

SKR_EXTERN_C SKR_RENDERER_API void skr_resolve_cameras_to_viewport(struct SViewportManager* viewport_manager, sugoi_storage_t* storage);
