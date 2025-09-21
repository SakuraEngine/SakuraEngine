#pragma once
// The SceneRenderSystem: A render system for rendering scene using SceneRenderer
#include "SkrRuntime/ecs/world.hpp"

#include "SkrRenderer/primitive_draw.h"
#include "scene_renderer.hpp"

namespace skr::ecs
{
struct World;
}

namespace skr
{
namespace render_graph
{
class RenderGraph;
}
} // namespace skr

namespace skr::scene
{

struct SCENE_RENDERER_API SceneRenderSystem
{
public:
    struct Context
    {
        skr::task::event_t update_finish;
    };
    static SceneRenderSystem* Create(skr::ecs::ECSWorld* world) SKR_NOEXCEPT;
    static void Destroy(SceneRenderSystem* system) SKR_NOEXCEPT;

    void bind_renderer(skr::SceneRenderer* renderer) SKR_NOEXCEPT;
    void update() SKR_NOEXCEPT;
    skr::Span<skr_primitive_draw_t> get_drawcalls() const SKR_NOEXCEPT;
    Context const* get_context() const SKR_NOEXCEPT;

private:
private:
    SceneRenderSystem() SKR_NOEXCEPT = default;
    ~SceneRenderSystem() SKR_NOEXCEPT = default;
    struct Impl;
    Impl* impl;

    // The render job for rendering meshes in the scene
    struct SceneRenderJob;
};

} // namespace skr::scene
