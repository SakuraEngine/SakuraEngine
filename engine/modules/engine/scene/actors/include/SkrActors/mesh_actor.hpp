#pragma once
#include "SkrScene/actor.hpp"
#include "SkrActors/mesh_actor.generated.h"

namespace skr
{

struct [[sattr(
    guid = "01987a21-a2b4-7488-924d-17639e937f87";
    rttr = @enable;
)]] SKR_ACTORS_API MeshActor : public Actor
{
    SKR_GENERATE_BODY(MeshActor)

    MeshActor() = default;
    ~MeshActor() override = default;
    void CreateDefaultComponents(ecs::ArchetypeBuilder& builder) override;
    void CreateDefaultEntity(ecs::TaskContext& ctx) override;
};

} // namespace skr