#include "SkrRTTR/type.hpp"
#include "SkrRTTR/type_registry.hpp"
#include "SkrSceneCore/scene_components.h"
#include "SkrRuntime/ecs/world.hpp"
#include "SkrRuntime/sugoi/sugoi_config.h"
#include "SkrScene/actor.h"
#include "SkrScene/actor_manager.h"

namespace skr
{

void Actor::Initialize()
{
    guid = skr::GUID::Create();
    Initialize(guid);
}

void Actor::Initialize(GUID _guid)
{
    this->guid = _guid;
    attach_rule = EAttachRule::Default;
    rttr_type_guid = skr::type_id_of<Actor>();
    display_name = u8"GeneralActor";
    spawner = skr::UPtr<Spawner>::New(
        [this](skr::ecs::ArchetypeBuilder& Builder) {
            Builder
                .add_component<skr::scene::ParentComponent>()
                .add_component<skr::scene::ChildrenComponent>()
                .add_component<skr::scene::PositionComponent>()
                .add_component<skr::scene::RotationComponent>()
                .add_component<skr::scene::ScaleComponent>()
                .add_component<skr::scene::TransformComponent>();
        },
        [this](skr::ecs::TaskContext& Context) {
            SkrZoneScopedN("Actor::Spawner::run");
            this->scene_entities.resize_zeroed(1);
            this->scene_entities[0] = Context.entities()[0];
            SKR_LOG_INFO(u8"Actor {%s} created with entity: {%u}", this->GetDisplayName().c_str(), this->GetEntity());
        }
    );
}

Actor::~Actor() SKR_NOEXCEPT
{
}

void Actor::DetachAllChildren()
{
    while (!children.is_empty())
    {
        auto child = children.back();
        child->DetachFromParent();
    }
}

void Actor::serialize() SKR_NOEXCEPT
{
    children_serialized.reserve(children.size());
    for (auto& child : children)
    {
        children_serialized.push_back(child->GetGUID());
    }
    parent_serialized = _parent ? _parent->GetGUID() : GUID::Zero();
    scene_entities_serialized.reserve(scene_entities.size());
    for (auto& entity : scene_entities)
    {
        scene_entities_serialized.push_back(sugoi_entity_t(entity));
    }
}

void Actor::deserialize() SKR_NOEXCEPT
{
    auto& manager = skr::ActorManager::GetInstance();
    children.reserve(children_serialized.size());
    for (auto& child_guid : children_serialized)
    {
        if (auto child = manager.GetActor(child_guid))
        {
            children.push_back(child.lock());
        }
    }
    if (!parent_serialized.is_zero())
    {
        if (auto parent = manager.GetActor(parent_serialized))
        {
            _parent = parent.lock();
        }
    }

    scene_entities.reserve(scene_entities_serialized.size());
    for (auto& entity : scene_entities_serialized)
    {
        scene_entities.push_back(skr::ecs::Entity(entity));
    }
    world = manager.GetRoot().lock()->GetWorld();
}

skr::RCWeak<RootActor> Actor::GetRoot()
{
    return ActorManager::GetInstance().GetRoot();
}

void Actor::CreateEntity()
{
    SKR_ASSERT(this); // TODO: 遇到过this指针为0的情况，有点担心出现UB，需要查明白
    skr::ActorManager::GetInstance().CreateActorEntity(this);
}

skr::ecs::Entity Actor::GetEntity() const
{
    if (scene_entities.is_empty())
    {
        SKR_LOG_ERROR(u8"Actor {%s} has no entity associated", display_name.c_str());
        return skr::ecs::Entity{ SUGOI_NULL_ENTITY };
    }
    return scene_entities[0];
}

void Actor::AttachTo(RCWeak<Actor> parent, EAttachRule rule)
{
    // if _parent is not null, detach from current parent
    if (_parent)
    {
        DetachFromParent();
    }
    parent.lock()->children.emplace(this);
    _parent = parent.lock();
    attach_rule = rule;
    // update hierarchy
    skr::ActorManager::GetInstance().UpdateHierarchy(_parent, this, rule);
}

void Actor::DetachFromParent()
{
    if (_parent)
    {
        // Remove this actor from parent's children list
        auto& siblings = _parent->children;
        siblings.remove_if([&](auto sibling) { return sibling.get() == this; });
        skr::ActorManager::GetInstance().UpdateHierarchy(_parent, this, attach_rule);
        _parent = nullptr; // Clear parent reference
    }
}

skr::Vector<skr::GUID> Actor::GetChildrenGUIDs() const SKR_NOEXCEPT
{
    skr::Vector<skr::GUID> children_guids;
    for (auto& child : children)
    {
        children_guids.push_back(child->GetGUID());
    }
    return children_guids;
}

/////////////////////
// ActorManager Implementation
/////////////////////

skr::RC<Actor> ActorManager::CreateActor(skr::GUID actor_rttr_guid)
{
    RTTRType* ActorType = skr::get_type_from_guid(actor_rttr_guid);
    void* actor_data = sakura_malloc_aligned(ActorType->size(), ActorType->alignment());
    ActorType->find_default_ctor().invoke(actor_data);
    return skr::RC<Actor>(reinterpret_cast<Actor*>(actor_data));
}

skr::RCWeak<Actor> ActorManager::GetActor(skr::GUID guid)
{
    return scene->actors.find(guid).value();
}

bool ActorManager::DestroyActor(skr::GUID guid)
{
    auto it = scene->actors.find(guid);
    if (!it)
    {
        SKR_LOG_FMT_ERROR(u8"Actor with GUID {{{}}} not found", guid);
        return false;
    }
    it.value()->DetachAllChildren();
    it.value()->DetachFromParent();
    DestroyActorEntity(it.value()); // Destroy the actor's entity in ECS world

    scene->actors.remove(guid); // when ref-counted -> 0, it will call SkrDelete with release()
    return true;
}

void ActorManager::CreateActorEntity(skr::RCWeak<Actor> actor)
{
    // check if actor's scene_entities is empty
    if (actor.lock()->scene_entities.is_empty())
    {
        world->create_entities(*actor.lock()->spawner, 1);
    }
    else
    {
        SKR_LOG_WARN(u8"Actor {%s} already has an entity associated", actor.lock()->GetDisplayName().c_str());
    }
}

void ActorManager::DestroyActorEntity(skr::RCWeak<Actor> actor)
{
    if (!actor.lock()->scene_entities.is_empty())
    {
        world->destroy_entities(actor.lock()->scene_entities);
        actor.lock()->scene_entities.clear(); // Clear the entity list after destruction
    }
    else
    {
        SKR_LOG_WARN(u8"Actor {%s} has no entities to destroy", actor.lock()->GetDisplayName().c_str());
    }
}

void ActorManager::UpdateHierarchy(skr::RCWeak<Actor> parent, skr::RCWeak<Actor> child, EAttachRule rule)
{
    auto parent_accessor = world->random_readwrite<skr::scene::ParentComponent>();
    auto children_accessor = world->random_readwrite<skr::scene::ChildrenComponent>();
    // update the parent/child components
    if (parent && child)
    {
        // child should not be root
        if (child.lock()->guid == skr::Actor::GetRoot().lock()->guid)
        {
            SKR_LOG_ERROR(u8"Cannot attach root actor as child");
            return;
        }
        auto parent_entity = parent.lock()->GetEntity();
        auto child_entity = child.lock()->GetEntity();
        if (parent_entity != skr::ecs::Entity{ SUGOI_NULL_ENTITY } && child_entity != skr::ecs::Entity{ SUGOI_NULL_ENTITY })
        {
            // update parent component
            parent_accessor.write_at(child_entity, skr::scene::ParentComponent{ parent_entity });
            skr::scene::ChildrenArray children;
            for (auto& new_child : parent.lock()->children)
            {
                children.push_back(skr::scene::ChildrenComponent{ .entity = new_child->GetEntity() });
            }
            // update children component
            children_accessor.write_at(parent_entity, children);
        }
        else
        {
            SKR_LOG_ERROR(u8"Parent or Child actor has no valid entity");
        }
    }
    else
    {
        SKR_LOG_ERROR(u8"Parent or Child actor is null");
    }
}

void ActorManager::ClearAllActors()
{
    // print out all keys

    auto root_guid = GetRoot().lock()->GetGUID();
    for (auto& actor_item : scene->actors)
    {
        // filter out root because it contains the world
        if (actor_item.key != root_guid)
        {
            DestroyActor(actor_item.key);
        }
    }
    DestroyActor(root_guid);
    scene->actors.clear();
}

skr::RCWeak<RootActor> ActorManager::GetRoot()
{
    static skr::RCWeak<RootActor> root = nullptr;
    if (!root)
    {
        if (!scene)
        {
            SKR_LOG_ERROR(u8"ActorManager::GetRoot: scene is null, Please bind scene first");
            return nullptr;
        }
        auto _root = CreateActorInstance<RootActor>();
        _root->Initialize(scene->root_actor_guid);
        scene->actors.add(_root->GetGUID(), _root);
        root = _root.cast_static<RootActor>();
    }
    return root; // Return the root actor reference
}

} // namespace skr
