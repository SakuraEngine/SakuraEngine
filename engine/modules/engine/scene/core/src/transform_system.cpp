#include "SkrBase/math.h"
#include "SkrSceneCore/scene_components.h"
#include "rtm/qvvf.h"
#include "SkrContainers/hashmap.hpp"
#include "SkrRuntime/ecs/world.hpp"
#include "SkrTask/parallel_for.hpp"
#include "SkrSceneCore/transform_system.h"

namespace skr::scene
{

inline bool check_dirty(const skr::scene::PositionComponent& position,
    const skr::scene::RotationComponent* rotation,
    const skr::scene::ScaleComponent* scale)
{
    const bool position_dirty = position.get_dirty();
    const bool rotation_dirty = rotation ? rotation->get_dirty() : false;
    const bool scale_dirty = scale ? scale->get_dirty() : false;
    return position_dirty || rotation_dirty || scale_dirty;
}

struct TransformFromRootJob
{
    void build(skr::ecs::AccessBuilder& builder)
    {
        // filter out root actor
        builder.none<scene::ParentComponent>()
            .has<scene::PositionComponent>()
            .has<scene::TransformComponent>();

        builder.access(&TransformFromRootJob::children_accessor)
            .access(&TransformFromRootJob::position_accessor)
            .access(&TransformFromRootJob::scale_accessor)
            .access(&TransformFromRootJob::rotation_accessor)
            .access(&TransformFromRootJob::transform_accessor);
    }

    void calculate(skr::ecs::Entity entity, const skr::scene::Transform& prev_transform, bool force_update = false)
    {
        auto pOptionalRotation = rotation_accessor.get(entity);
        auto pOptionalScale = scale_accessor.get(entity);
        const auto& Position = position_accessor[entity];

        const bool dirty = force_update || check_dirty(Position, pOptionalRotation, pOptionalScale);
        auto& transform = transform_accessor[entity];
        // TODO: 当前的更新逻辑存在问题
        // 1. 对于根节点，需要手动设置Position来触发更新，否则会默认初始化为0
        // 2. 对于某些希望保持世界变换的节点更新逻辑没有照顾到
        if (dirty)
        {
            transform.set(
                Position.get(),
                pOptionalRotation ? skr::QuatF(pOptionalRotation->get()) : skr::QuatF(0, 0, 0, 1),
                pOptionalScale ? pOptionalScale->get() : float3{ 1, 1, 1 });
            transform.set(prev_transform * transform.get());

            Position.dirty = false;
            if (pOptionalRotation)
                pOptionalRotation->dirty = false;
            if (pOptionalScale)
                pOptionalScale->dirty = false;
        }

        for (const auto& child : children_accessor[entity])
        {
            calculate(child.entity, transform.get(), dirty || force_update);
        }
    }

    void run(skr::ecs::TaskContext& Context)
    {
        SkrZoneScopedN("UpdateTransforms");
        for (auto i = 0; i < Context.size(); i++)
        {
            const auto entity = Context.entities()[i];
            calculate(entity, skr::scene::Transform::Identity());
        }
    }

    skr::ecs::RandomComponentReader<const skr::scene::ChildrenComponent> children_accessor;
    skr::ecs::RandomComponentReader<const skr::scene::PositionComponent> position_accessor;
    skr::ecs::RandomComponentReader<const skr::scene::ScaleComponent> scale_accessor;
    skr::ecs::RandomComponentReader<const skr::scene::RotationComponent> rotation_accessor;
    skr::ecs::RandomComponentReadWrite<skr::scene::TransformComponent> transform_accessor;
};

} // namespace skr::scene

namespace skr
{

struct skr::TransformSystem::Impl
{
    skr::ecs::ECSWorld* pWorld = nullptr;
    sugoi_query_t* rootJobQuery = nullptr;
    skr::TransformSystem::Context context;
};

TransformSystem* TransformSystem::Create(skr::ecs::ECSWorld* world) SKR_NOEXCEPT
{
    SkrZoneScopedN("CreateTransformSystem");
    auto memory = (uint8_t*)sakura_calloc(1, sizeof(TransformSystem) + sizeof(TransformSystem::Impl));
    auto system = new (memory) TransformSystem();
    system->impl = new (memory + sizeof(TransformSystem)) TransformSystem::Impl();
    system->impl->pWorld = world;
    return system;
}

void TransformSystem::Destroy(TransformSystem* system) SKR_NOEXCEPT
{
    SkrZoneScopedN("FinalizeTransformSystem");
    system->impl->~Impl();
    system->~TransformSystem();
    sakura_free(system);
}

TransformSystem::Context const* TransformSystem::get_context() const SKR_NOEXCEPT
{
    return &impl->context;
}

void TransformSystem::update() SKR_NOEXCEPT
{
    CalculateFromRoot();
}

void TransformSystem::CalculateFromRoot() SKR_NOEXCEPT
{
    impl->context.update_finish.clear();
    skr::ecs::TaskOptions options;
    options.on_finishes.add(impl->context.update_finish);
    scene::TransformFromRootJob job;
    impl->rootJobQuery = impl->pWorld->dispatch_task(job, UINT32_MAX, impl->rootJobQuery, std::move(options));
}

void TransformSystem::CalculateTransform(sugoi_entity_t entity) SKR_NOEXCEPT
{
    SkrZoneScopedN("CalculateTransform");
    auto parent_accessor = impl->pWorld->random_read<const skr::scene::ParentComponent>();
    auto position_accessor = impl->pWorld->random_read<const skr::scene::PositionComponent>();
    auto rotation_accessor = impl->pWorld->random_read<const skr::scene::RotationComponent>();
    auto scale_accessor = impl->pWorld->random_read<const skr::scene::ScaleComponent>();
    auto children_accessor = impl->pWorld->random_read<const skr::scene::ChildrenComponent>();
    auto transform_accessor = impl->pWorld->random_readwrite<skr::scene::TransformComponent>();

    skr::ecs::Entity ent{ entity };
    auto check_dirty_ent = [&](skr::ecs::Entity ent) {
        auto pOptionalRotation = rotation_accessor.get(ent);
        auto pOptionalScale = scale_accessor.get(ent);
        const auto& Position = position_accessor[ent];
        return check_dirty(Position, pOptionalRotation, pOptionalScale);
    };

    std::function<void(skr::ecs::Entity, const skr::scene::Transform&)> update_transform;
    update_transform = [&](skr::ecs::Entity _ent, const skr::scene::Transform& prev_transform) {
        auto pOptionalRotation = rotation_accessor.get(_ent);
        auto pOptionalScale = scale_accessor.get(_ent);
        const auto& Position = position_accessor[_ent];

        const bool dirty = check_dirty(Position, pOptionalRotation, pOptionalScale);
        auto& transform = transform_accessor[_ent];
        if (dirty)
        {
            transform.set(
                Position.get(),
                pOptionalRotation ? skr::QuatF(pOptionalRotation->get()) : skr::QuatF(0, 0, 0, 1),
                pOptionalScale ? pOptionalScale->get() : float3{ 1, 1, 1 });
            transform.set(prev_transform * transform.get());

            Position.dirty = false;
            if (pOptionalRotation)
                pOptionalRotation->dirty = false;
            if (pOptionalScale)
                pOptionalScale->dirty = false;
        }

        for (const auto& child : children_accessor[_ent])
        {
            update_transform(child.entity, transform.get());
        }
    };

    bool dirty = check_dirty_ent(ent);
    const scene::ParentComponent* pParent = nullptr;
    if (dirty)
    {
        do
        {
            pParent = parent_accessor.get(ent);
            if (!pParent)
            {
                // if no parent, use the root transform
                CalculateFromRoot();
                return;
            }
            ent = pParent->entity;
        } while ((ent != skr::ecs::Entity{ SUGOI_NULL_ENTITY } && check_dirty_ent(ent)));
    }

    // now we have the first parent that is not dirty, calculate the transform
    auto transform = transform_accessor.get(ent);
    for (const auto& child : children_accessor[ent])
    {
        update_transform(child.entity, transform->get());
    }
}

} // namespace skr

skr::TransformSystem* skr_transform_system_create(skr::ecs::ECSWorld* world)
{
    return skr::TransformSystem::Create(world);
}

void skr_transform_system_destroy(skr::TransformSystem* system)
{
    skr::TransformSystem::Destroy(system);
}

void skr_transform_system_update(skr::TransformSystem* system)
{
    system->update();
}