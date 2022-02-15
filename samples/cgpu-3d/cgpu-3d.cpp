#include "render-device.hpp"
#include "render-context.hpp"
#include "render-scene.hpp"
#include "math/scalarmath.h"
#include "math/vectormath.hpp"
#include <EASTL/unique_ptr.h>

int main(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) return -1;
    auto render_device = eastl::make_unique<RenderDevice>();
    render_device->Initialize(CGPU_BACKEND_VULKAN);
    auto render_context = eastl::make_unique<RenderContext>();
    render_context->Initialize(render_device.get());
    auto render_scene = eastl::make_unique<RenderScene>();
    render_scene->Initialize("./../Resources/scene.gltf");
    render_scene->Upload(render_context.get());
    render_scene->Destroy();
    render_context->Destroy();
    render_device->Destroy();
    return 0;
}