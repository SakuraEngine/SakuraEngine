#pragma once
#include "SkrLive2D/skr_live2d.configure.h"
#include "platform/configure.h"

#ifdef __cplusplus
struct SRenderer;
namespace skr { namespace render_graph { class RenderGraph; } }
using live2d_render_graph_t = skr::render_graph::RenderGraph;
using live2d_renderer_t = SRenderer;
#else
typedef struct skr_render_graph_t live2d_render_graph_t;
typedef struct SRenderer live2d_renderer_t;
#endif

typedef struct live2d_render_view_t live2d_render_view_t;
typedef live2d_render_view_t* live2d_render_view_id;

SKR_LIVE2D_EXTERN_C SKR_LIVE2D_API
live2d_render_view_id skr_live2d_create_render_view();

SKR_LIVE2D_EXTERN_C SKR_LIVE2D_API
void skr_live2d_render_view_reset(live2d_render_view_id view);

SKR_LIVE2D_EXTERN_C SKR_LIVE2D_API
void skr_live2d_render_view_set_screen(live2d_render_view_id view, uint32_t width, uint32_t height);

SKR_LIVE2D_EXTERN_C SKR_LIVE2D_API
void skr_live2d_render_view_transform_screen(live2d_render_view_id view, float deviceX, float deviceY);

SKR_LIVE2D_EXTERN_C SKR_LIVE2D_API
void skr_live2d_render_view_transform_view(live2d_render_view_id view, float deviceX, float deviceY);

SKR_LIVE2D_EXTERN_C SKR_LIVE2D_API
void skr_live2d_free_render_view(live2d_render_view_id view);

SKR_LIVE2D_EXTERN_C SKR_LIVE2D_API
void skr_live2d_initialize_render_effects(live2d_renderer_t* renderer, live2d_render_graph_t* render_graph, struct skr_vfs_t* resource_vfs);

SKR_LIVE2D_EXTERN_C SKR_LIVE2D_API
void skr_live2d_finalize_render_effects(live2d_renderer_t* renderer, live2d_render_graph_t* render_graph, struct skr_vfs_t* resource_vfs);