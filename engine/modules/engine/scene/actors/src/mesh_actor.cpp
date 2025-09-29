#include "SkrActors/mesh_actor.hpp"
#include "SkrRenderer/render_mesh.h"
#include "SkrScene/basic_components.hpp"
#include "SkrRenderer/gpu_scene.h"
#include "SkrRenderer/shared/gpu_scene.hpp"

namespace skr
{

void MeshActor::CreateDefaultComponents(ecs::ArchetypeBuilder& builder)
{
    Super::CreateDefaultComponents(builder);
    builder
        .add_component<skr::gpu::Instance>()
        .add_component<skr::GPUSceneInstance>()
        .add_component<skr::MeshComponent>();
}

void MeshActor::CreateDefaultEntity(ecs::TaskContext& ctx)
{
    Super::CreateDefaultEntity(ctx);
    auto& gpu_insts = ctx.components<skr::gpu::Instance>()[0];
    gpu_insts.resize_default(1);
}

} // namespace skr