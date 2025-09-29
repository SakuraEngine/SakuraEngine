#pragma once
#include "SkrActors/mesh_actor.hpp"
#include "SkrActors/skeleton_mesh_actor.generated.h"

namespace skr
{

struct [[sattr(
    guid = "01987a21-e796-76b6-89c4-fb550edf5610";
    rttr = @enable;
)]] SKR_ACTORS_API SkeletonMeshActor : public MeshActor
{
    SKR_GENERATE_BODY(SkeletonMeshActor)

    SkeletonMeshActor() = default;
    ~SkeletonMeshActor() override = default;
    void CreateDefaultComponents(ecs::ArchetypeBuilder& builder) override;
};

} // namespace skr