#include "SkrAnim/components/skeleton_component.hpp"
#include "SkrAnim/components/skin_component.hpp"
#include "SkrRenderer/render_mesh.h"
#include "SkrSceneCore/scene_components.h"
#include "SkrScene/actor.h"

namespace skr
{

SkelMeshActor::~SkelMeshActor() SKR_NOEXCEPT
{

}

void SkelMeshActor::Initialize()
{
    Initialize(skr::GUID::Create());
}

void SkelMeshActor::Initialize(GUID _guid)
{
    attach_rule = EAttachRule::Default;
    guid = _guid;
    rttr_type_guid = skr::type_id_of<SkelMeshActor>();
    display_name = u8"SkelMeshActor";
    // override spawner
    spawner = skr::UPtr<Spawner>::New(
        [this](skr::ecs::ArchetypeBuilder& Builder) {
            Builder
                .add_component<skr::scene::ParentComponent>()
                .add_component<skr::scene::ChildrenComponent>()
                .add_component<skr::scene::PositionComponent>()
                .add_component<skr::scene::RotationComponent>()
                .add_component<skr::scene::ScaleComponent>()
                .add_component<skr::scene::TransformComponent>()
                .add_component<skr::MeshComponent>()
                .add_component<skr::SkeletonComponent>()
                .add_component<skr::AnimComponent>()
                .add_component<skr::SkinComponent>();
        },
        [this](skr::ecs::TaskContext& Context) {
            SkrZoneScopedN("SkelMeshActor::Spawner::run");
            this->scene_entities.resize_zeroed(1);
            this->scene_entities[0] = Context.entities()[0];
            SKR_LOG_INFO(u8"SkelMeshActor {%s} created with entity: {%u}", this->GetDisplayName().c_str(), this->GetEntity());
        });
}

} // namespace skr