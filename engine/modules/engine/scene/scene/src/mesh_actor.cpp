#include "SkrRenderer/render_mesh.h"
#include "SkrSceneCore/scene_components.h"
#include "SkrScene/actor.h"
#include "SkrRenderer/gpu_scene.h"
#include "SkrRenderer/shared/gpu_scene.hpp"

namespace skr
{

MeshActor::~MeshActor() SKR_NOEXCEPT
{

}

void MeshActor::Initialize()
{
    Initialize(skr::GUID::Create());
}

void MeshActor::Initialize(GUID _guid)
{
    this->guid = _guid;
    attach_rule = EAttachRule::Default;
    display_name = u8"MeshActor";
    rttr_type_guid = skr::type_id_of<MeshActor>();
    spawner = skr::UPtr<Spawner>::New(
        [this](skr::ecs::ArchetypeBuilder& Builder) {
            Builder.add_component<skr::scene::ParentComponent>()
                .add_component<skr::scene::ChildrenComponent>()
                .add_component<skr::scene::PositionComponent>()
                .add_component<skr::scene::RotationComponent>()
                .add_component<skr::scene::ScaleComponent>()
                .add_component<skr::scene::TransformComponent>()
                .add_component<skr::gpu::Instance>()
                .add_component<skr::GPUSceneInstance>()
                .add_component<skr::MeshComponent>();
        },
        [this](skr::ecs::TaskContext& Context) {
            SkrZoneScopedN("MeshActor::Spawner::run");
            this->scene_entities.resize_zeroed(1);
            this->scene_entities[0] = Context.entities()[0];
            
            auto& gpu_insts = Context.components<skr::gpu::Instance>()[0];
            gpu_insts.resize(1);

            SKR_LOG_INFO(u8"MeshActor {%s} created with entity: {%u}", this->GetDisplayName().c_str(), this->GetEntity());
        }
    );
}

} // namespace skr