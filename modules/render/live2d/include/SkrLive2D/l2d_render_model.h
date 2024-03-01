#pragma once
#include "SkrRT/misc/types.h"
#include "SkrRenderer/primitive_draw.h"
#include "l2d_model_resource.h"

typedef struct skr_live2d_clipping_manager_t  skr_live2d_clipping_manager_t;
typedef struct skr_live2d_clipping_manager_t* skr_live2d_clipping_manager_id;
typedef struct skr_live2d_render_model_t      skr_live2d_render_model_t;
typedef struct skr_live2d_render_model_t*     skr_live2d_render_model_id;

typedef struct skr_live2d_render_model_future_t {
    const char*                model_name;
    skr_vfs_t*                 vfs_override;
    CGPUQueueId                queue_override;
    ECGPUDStorageSource        dstorage_source;
    skr_live2d_render_model_id render_model;
    SAtomicU32                 io_status;
    bool                       use_dynamic_buffer;
#ifdef __cplusplus
    SKR_LIVE2D_API bool        is_ready() const SKR_NOEXCEPT;
    SKR_LIVE2D_API ESkrIOStage get_status() const SKR_NOEXCEPT;
#endif
} skr_live2d_render_model_future_t;

struct skr_live2d_render_model_comp_t {
    skr_guid_t                       resource_guid;
    skr_live2d_ram_io_future_t       ram_future;
    skr_live2d_render_model_future_t vram_future;
};
typedef struct skr_live2d_render_model_comp_t skr_live2d_render_model_comp_t;

#ifndef SKR_SERIALIZE_GURAD
SKR_LIVE2D_EXTERN_C SKR_LIVE2D_API void
skr_live2d_render_model_create_from_raw(skr_io_ram_service_t*, skr_io_vram_service_t*, CGPUDeviceId device,
                                        skr_live2d_model_resource_id resource, skr_live2d_render_model_future_t* request);
#endif

SKR_LIVE2D_EXTERN_C SKR_LIVE2D_API CGPUTextureId
skr_live2d_render_model_get_texture(skr_live2d_render_model_id render_model, uint32_t drawable_index);

SKR_LIVE2D_EXTERN_C SKR_LIVE2D_API CGPUTextureViewId
skr_live2d_render_model_get_texture_view(skr_live2d_render_model_id render_model, uint32_t drawable_index);

SKR_LIVE2D_EXTERN_C SKR_LIVE2D_API void
skr_live2d_render_model_free(skr_live2d_render_model_id render_model);

#ifdef __cplusplus
    #include "SkrRT/io/vram_io.hpp"
    #include "SkrContainers/map.hpp"
    #include "SkrContainers/vector.hpp"

struct skr_live2d_render_model_t {
    virtual ~skr_live2d_render_model_t() = default;
    skr_live2d_model_resource_id model_resource_id;
    bool                         use_dynamic_buffer = true;

    // clipping
    skr_live2d_clipping_manager_id clipping_manager;
    // pos-uv-pos-uv...
    skr::Vector<CGPUTextureId>                   textures;
    skr::Vector<CGPUTextureViewId>               texture_views;
    skr::Vector<skr::renderer::VertexBufferView> vertex_buffer_views;
    skr::Vector<skr::renderer::IndexBufferView>  index_buffer_views;
    skr::Vector<skr::renderer::PrimitiveCommand> primitive_commands;
    // bind table cache
    skr::Map<CGPUTextureViewId, CGPUXBindTableId> bind_tables;
    skr::Map<CGPUTextureViewId, CGPUXBindTableId> mask_bind_tables;
};

    #include "SkrRT/ecs/sugoi.h"
template <>
struct SKR_LIVE2D_API sugoi_id_of<skr_live2d_render_model_comp_t> {
    static sugoi_type_index_t get();
};
#endif
