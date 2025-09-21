#pragma once
#include "SkrContainersDef/span.hpp"
#include "SkrRenderer/primitive_draw.h"
#include "helper.hpp"

struct sugoi_storage_t;
namespace skr
{
struct RenderDevice;
}
namespace skr
{
namespace render_graph
{
class RenderGraph;
}
} // namespace skr
namespace skr
{
namespace ecs
{
class World;
}
} // namespace skr

namespace skr
{

struct SCENE_RENDERER_API SceneRenderer
{
    static SceneRenderer* Create();
    static void Destroy(SceneRenderer* renderer);

    virtual ~SceneRenderer();
    virtual void initialize(skr::RenderDevice* render_device, skr::ecs::ECSWorld* storage, struct skr_vfs_t* resource_vfs) = 0;
    virtual void finalize(skr::RenderDevice* renderer) = 0;

    virtual void draw_primitives(skr::render_graph::RenderGraph* render_graph, skr::Span<skr_primitive_draw_t> drawcalls) = 0;

    virtual void set_camera(utils::Camera* camera) = 0;
    virtual utils::Camera* get_camera() const { return nullptr; }
    virtual CGPURenderPipelineId get_render_pso() const = 0;
};
} // namespace skr
