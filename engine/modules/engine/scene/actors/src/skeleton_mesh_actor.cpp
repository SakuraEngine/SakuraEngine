#include "SkrActors/skeleton_mesh_actor.hpp"
#include "SkrAnim/components/skeleton_component.hpp"
#include "SkrAnim/components/skin_component.hpp"
#include "SkrRenderer/render_mesh.h"
#include "SkrScene/basic_components.hpp"

namespace skr
{
void SkeletonMeshActor::CreateDefaultComponents(ecs::ArchetypeBuilder& builder)
{
    Super::CreateDefaultComponents(builder);
    builder
        .add_component<skr::MeshComponent>()
        .add_component<skr::SkeletonComponent>()
        .add_component<skr::AnimComponent>()
        .add_component<skr::SkinComponent>();
}

} // namespace skr