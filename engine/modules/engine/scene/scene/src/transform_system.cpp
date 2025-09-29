#include "SkrBase/math.hpp"
#include "SkrScene/basic_components.hpp"
#include "rtm/qvvf.h"
#include "SkrContainers/hashmap.hpp"
#include "SkrRuntime/ecs/world.hpp"
#include "SkrTask/parallel_for.hpp"
#include "SkrScene/transform_system.hpp"

namespace skr
{

struct TransformFromRootJob
{
    void build(skr::ecs::AccessBuilder& builder)
    {
        // filter out root actor
        builder
            .has<ParentComponent>()
            .has<ChildrenComponent>()
            .has<PositionComponent>()
            .has<SolvedTransformComponent>();

        builder
            .access(&TransformFromRootJob::parent_accessor)
            .access(&TransformFromRootJob::children_accessor)
            .access(&TransformFromRootJob::position_accessor)
            .access(&TransformFromRootJob::scale_accessor)
            .access(&TransformFromRootJob::rotation_accessor)
            .access(&TransformFromRootJob::solved_transform_accessor);
    }

    void run(skr::ecs::TaskContext& Context)
    {
        SkrZoneScopedN("UpdateTransforms");

        for (auto i = 0; i < Context.size(); i++)
        {
            // filter out non-root actor
            const auto entity = Context.entities()[i];
            const auto& parent = parent_accessor[entity];
            if (parent.entity) continue;

            // calculate from root
            calc_single(entity, skr::Transform::Identity());
        }
    }

    void calc_single(skr::ecs::Entity entity, const skr::Transform& prev_transform, bool force_update = false)
    {
        // find components
        const auto* p_rotation = rotation_accessor.get(entity);
        const auto* p_scale = scale_accessor.get(entity);
        const auto* p_position = position_accessor.get(entity);
        auto* p_transform = solved_transform_accessor.get(entity);
        SKR_ASSERT(p_position && p_transform && "these components must be exist");

        // check dirty
        bool dirty =
            force_update ||
            (p_position && p_position->is_dirty()) ||
            (p_rotation && p_rotation->is_dirty()) ||
            (p_scale && p_scale->is_dirty());

        // calculate self
        if (dirty)
        {
            // calculate
            Transform curr_transform{
                p_rotation ? Quat{ p_rotation->get() } : Quat::Identity(),
                p_position->get(),
                p_scale ? p_scale->get() : float3{ 1 }
            };
            p_transform->set(prev_transform * curr_transform);

            // cancel dirty
            p_position->cancel_dirty();
            if (p_rotation) p_rotation->cancel_dirty();
            if (p_scale) p_scale->cancel_dirty();
        }

        // calculate children
        for (const auto& child : children_accessor[entity])
        {
            calc_single(
                child.entity,
                p_transform->get(),
                force_update
            );
        }
    }

    skr::ecs::RandomComponentReader<const skr::ParentComponent> parent_accessor;
    skr::ecs::RandomComponentReader<const skr::ChildrenComponent> children_accessor;
    skr::ecs::RandomComponentReader<const skr::PositionComponent> position_accessor;
    skr::ecs::RandomComponentReader<const skr::ScaleComponent> scale_accessor;
    skr::ecs::RandomComponentReader<const skr::RotationComponent> rotation_accessor;
    skr::ecs::RandomComponentReadWrite<skr::SolvedTransformComponent> solved_transform_accessor;
};

} // namespace skr

namespace skr
{

struct skr::TransformSystem::Impl
{
    skr::ecs::ECSWorld* pWorld = nullptr;
    sugoi_query_t* rootJobQuery = nullptr;
    skr::task::event_t update_finish = {};
};

UPtr<TransformSystem> TransformSystem::Create(skr::ecs::ECSWorld* world)
{
    SkrZoneScopedN("CreateTransformSystem");
    auto memory = (uint8_t*)sakura_calloc(1, sizeof(TransformSystem) + sizeof(TransformSystem::Impl));
    auto system = new (memory) TransformSystem();
    system->impl = new (memory + sizeof(TransformSystem)) TransformSystem::Impl();
    system->impl->pWorld = world;
    return { system };
}

void TransformSystem::Destroy(TransformSystem* system)
{
    SkrZoneScopedN("FinalizeTransformSystem");
    system->impl->~Impl();
    system->~TransformSystem();
    sakura_free(system);
}

void TransformSystem::update()
{
    CalculateFromRoot();
}
void TransformSystem::wait()
{
    impl->update_finish.wait(true);
}

void TransformSystem::CalculateFromRoot()
{
    // setup options
    impl->update_finish.clear();
    skr::ecs::TaskOptions options;
    options.on_finishes.add(impl->update_finish);

    // emit job
    TransformFromRootJob job;
    impl->rootJobQuery = impl->pWorld->dispatch_task(
        job,
        UINT32_MAX,
        impl->rootJobQuery,
        std::move(options)
    );
}
} // namespace skr
