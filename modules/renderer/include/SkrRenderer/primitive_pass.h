#pragma once
#include "SkrRenderer/module.configure.h"
#include "SkrRenderer/primitive_draw.h"

namespace skr
{
namespace render_graph
{
class RenderGraph;
}
} // namespace skr

struct dual_storage_t;
typedef const char* skr_render_pass_name_t;

typedef struct skr_primitive_pass_context_t {
    SRendererId renderer;
    skr::render_graph::RenderGraph* render_graph;
    struct dual_storage_t* storage;
} skr_primitive_pass_context_t;

struct IPrimitiveRenderPass {
#ifdef __cplusplus
    virtual ~IPrimitiveRenderPass() = default;

    virtual void on_update(const skr_primitive_pass_context_t* context) = 0;
    virtual void post_update(const skr_primitive_pass_context_t* context) = 0;

    virtual void execute(const skr_primitive_pass_context_t* context, skr_primitive_draw_list_view_t dc) = 0;
    virtual skr_render_pass_name_t identity() const = 0;
#endif
};

#ifdef __cplusplus
extern "C" {
#endif

SKR_RENDERER_EXTERN_C SKR_RENDERER_API void
skr_renderer_register_render_pass(SRendererId renderer, skr_render_pass_name_t name, IPrimitiveRenderPass* pass);

SKR_RENDERER_EXTERN_C SKR_RENDERER_API void
skr_renderer_remove_render_pass(SRendererId renderer, skr_render_pass_name_t name);

#ifdef __cplusplus
}
#endif