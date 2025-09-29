#include "SkrRTTR/type.hpp"
#include "SkrRTTR/type_registry.hpp"
#include "SkrScene/basic_components.hpp"
#include "SkrRuntime/ecs/world.hpp"
#include "SkrRuntime/sugoi/sugoi_config.h"
#include "SkrScene/actor.hpp"
#include <SkrScene/world.hpp>

namespace skr
{

// ctor & dtor
Actor::Actor()
{
}
Actor::~Actor()
{
}

// world
ecs::ECSWorld* Actor::GetECSWorld() const
{
    SKR_ASSERT(_world && "World is null, Please bind world first");
    return _world->GetECSWorld();
}
World* Actor::GetWorld() const
{
    SKR_ASSERT(_world && "World is null, Please bind world first");
    return _world;
}

// transform management
void Actor::SetLocalPosition(const Position& position)
{
    auto position_comp = FindComponentUnsafe<skr::PositionComponent>();
    SKR_ASSERT(position_comp);
    position_comp->set(position);
}
void Actor::SetLocalRotation(const Rotator& rotation)
{
    auto rotation_comp = FindComponentUnsafe<skr::RotationComponent>();
    SKR_ASSERT(rotation_comp);
    rotation_comp->set(rotation);
}
void Actor::SetLocalScale(const Scale& scale)
{
    auto scale_comp = FindComponentUnsafe<skr::ScaleComponent>();
    SKR_ASSERT(scale_comp);
    scale_comp->set(scale);
}
Position Actor::GetLocalPosition() const
{
    auto position_comp = FindComponentUnsafe<skr::PositionComponent>();
    SKR_ASSERT(position_comp);
    return position_comp->get();
}
Rotator Actor::GetLocalRotation() const
{
    auto rotation_comp = FindComponentUnsafe<skr::RotationComponent>();
    SKR_ASSERT(rotation_comp);
    return rotation_comp->get();
}
Scale Actor::GetLocalScale() const
{
    auto scale_comp = FindComponentUnsafe<skr::ScaleComponent>();
    SKR_ASSERT(scale_comp);
    return scale_comp->get();
}
Transform Actor::GetWorldTransform() const
{
    auto transform_comp = FindComponentUnsafe<skr::SolvedTransformComponent>();
    SKR_ASSERT(transform_comp);
    return transform_comp->get();
}

// hierarchy
void Actor::Attach(Actor* parent)
{
    SKR_ASSERT(parent && "Parent is null");
    if (_parent == parent) return; // fast skip if already attached to the same parent
    if (_parent) Detach();         // detach first

    // update parent
    _parent = parent;
    parent->_children.push_back(this);

    // update hierarchy in ecs
    {
        auto parent_accessor = GetECSWorld()->random_readwrite<skr::ParentComponent>();
        auto children_accessor = GetECSWorld()->random_readwrite<skr::ChildrenComponent>();
        auto parent_children = children_accessor.get(parent->GetDefaultEntity());
        auto self_parent = parent_accessor.get(GetDefaultEntity());
        parent_children->add({ GetDefaultEntity() });
        self_parent->entity = parent->GetDefaultEntity();
    }
}
void Actor::Detach()
{
    if (!_parent) return; // fast skip if no parent

    // remove from parent's children list
    _parent->_children.remove(this);

    // update hierarchy in ecs
    {
        auto parent_accessor = GetECSWorld()->random_readwrite<skr::ParentComponent>();
        auto children_accessor = GetECSWorld()->random_readwrite<skr::ChildrenComponent>();
        auto parent_children = children_accessor.get(_parent->GetDefaultEntity());
        auto self_parent = parent_accessor.get(GetDefaultEntity());
        parent_children->remove_if([this](const skr::ChildrenComponent& child) {
            return child.entity == GetDefaultEntity();
        });
        self_parent->entity = {};
    }

    // clear parent
    _parent = nullptr;
}

// destroy
void Actor::Destroy()
{
    GetWorld()->DestroyActor(this);
}

// init call backs
void Actor::OnInitialize()
{
}
void Actor::CreateDefaultComponents(ecs::ArchetypeBuilder& builder)
{
    builder
        // hierarchy
        .add_component<skr::ParentComponent>()
        .add_component<skr::ChildrenComponent>()
        // transform
        .add_component<skr::PositionComponent>()
        .add_component<skr::RotationComponent>()
        .add_component<skr::ScaleComponent>()
        .add_component<skr::SolvedTransformComponent>();
}
void Actor::CreateDefaultEntity(ecs::TaskContext& ctx)
{
    _default_entity = ctx.entities()[0];
}
void Actor::BeginPlay()
{
}
bool Actor::IsNeverTick() const
{
    return true;
}
void Actor::OnTick(double delta_time)
{
}
void Actor::EndPlay()
{
}
void Actor::BeginDestroy()
{
}

// called by world
void Actor::_Initialize(GUID guid, World* world)
{
    _guid = guid;
    _world = world;
}
void Actor::_DestroyEntity()
{
    SKR_ASSERT(_world && "World is null, Please bind world first");
    if (_default_entity)
    {
        GetECSWorld()->destroy_entities({ &_default_entity, 1 });
        _default_entity = ecs::Entity{};
    }
}
} // namespace skr
