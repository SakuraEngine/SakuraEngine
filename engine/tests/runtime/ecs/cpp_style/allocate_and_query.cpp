#include "cpp_style.hpp"
#include "SkrTask/parallel_for.hpp"
#include "SkrRuntime/ecs/world.hpp"
#include "SkrRuntime/ecs/query.hpp"
#include "SkrRuntime/sugoi/storage.hpp"

struct AllocateEntites
{
    AllocateEntites() SKR_NOEXCEPT
    {
        world.initialize();
        storage = world.get_storage();
    }
    ~AllocateEntites() SKR_NOEXCEPT
    {
        world.finalize();
    }
    sugoi_storage_t* storage = nullptr;
    skr::ecs::ECSWorld world;
};

static constexpr size_t kIntEntityCount = 12;
static constexpr size_t kFloatEntityCount = 12;
static constexpr size_t kBothEntityCount = 12;
skr::ecs::Entity shared_entity = skr::ecs::Entity(SUGOI_NULL_ENTITY);

template <typename... Ts>
struct EntitySpawner
{
    using F = std::function<void(skr::ecs::TaskContext&)>;
    EntitySpawner(F func, sugoi_entity_t shared = SUGOI_NULL_ENTITY)
        : f(func), meta_ent(shared)
    {
    }

    void build(skr::ecs::ArchetypeBuilder& Builder)
    {
        (Builder.add_component<Ts>(), ...);
        if (meta_ent != SUGOI_NULL_ENTITY)
            Builder.add_meta_entity(meta_ent);
    }

    void run(skr::ecs::TaskContext& Context)
    {
        f(Context);
    }
    
    skr::ecs::Entity meta_ent = skr::ecs::Entity(SUGOI_NULL_ENTITY);
    F f;
};

TEST_CASE_METHOD(AllocateEntites, "AllocateAndQuery")
{
    SkrZoneScopedN("AllocateEntites::AllocateAndQuery");

    {
        auto spawner = EntitySpawner<SharedComponent>(
            [&](skr::ecs::TaskContext& ctx) {
                auto pcomp = ctx.components<SharedComponent>();
                pcomp[0].i = 114;
                pcomp[0].f = 514.f;
                EXPECT_EQ(ctx.size(), 1);
                shared_entity = ctx.entities()[0];
            });
        world.create_entities(spawner, 1);
    }
    {
        auto spawner = EntitySpawner<IntComponent>(
            [&](skr::ecs::TaskContext& ctx) {
                auto ints = ctx.components<IntComponent>();
                for (auto i = 0; i < ctx.size(); i++)
                {
                    ints[i].v = i;
                }
                EXPECT_EQ(ctx.size(), kIntEntityCount);
            });
        world.create_entities(spawner, kIntEntityCount);
    }
    {
        auto spawner = EntitySpawner<FloatComponent>(
            [&](skr::ecs::TaskContext& ctx) {
                auto floats = ctx.components<FloatComponent>();
                for (auto i = 0; i < ctx.size(); i++)
                {
                    floats[i].v = i * 2.f;
                }
                EXPECT_EQ(ctx.size(), kFloatEntityCount);
            });
        world.create_entities(spawner, kFloatEntityCount);
    }
    {
        auto spawner = EntitySpawner<IntComponent, FloatComponent>
        ([&](skr::ecs::TaskContext& ctx) {
            auto ints = ctx.components<IntComponent>();
            auto floats = ctx.components<FloatComponent>();
            for (auto i = 0; i < ctx.size(); i++)
            {
                ints[i].v = i;
                floats[i].v = i * 2.f;
            }
            EXPECT_EQ(ctx.size(), kBothEntityCount);
        }, (sugoi_entity_t)shared_entity);
        world.create_entities(spawner, kBothEntityCount);
    }
    // ReadAll
    {
        auto q = skr::ecs::QueryBuilder(&world)
                     .ReadAll<FloatComponent, IntComponent>()
                     .commit();
        SKR_DEFER({ sugoiQ_release(q.value()); });
        EXPECT_OK(q);
        auto callback = [&](sugoi_chunk_view_t* view) {
            auto ints = sugoi::get_owned<const IntComponent>(view);
            auto floats = sugoi::get_owned<const FloatComponent>(view);
            for (auto i = 0; i < view->count; i++)
            {
                EXPECT_EQ(ints[i].v, i);
                EXPECT_EQ(floats[i].v, i * 2.f);
            }

            // can't get writable comps with readonly query signature
            EXPECT_EQ(sugoi::get_owned<IntComponent>(view), nullptr);
            EXPECT_EQ(sugoi::get_owned<FloatComponent>(view), nullptr);
        };
        sugoiQ_get_views(q.value(), SUGOI_LAMBDA(callback));
    }
    // ReadWriteAll
    {
        auto q = skr::ecs::QueryBuilder(&world)
                     .ReadWriteAll<FloatComponent, IntComponent>()
                     .commit();
        SKR_DEFER({ sugoiQ_release(q.value()); });
        EXPECT_OK(q);
        auto callback = [&](sugoi_chunk_view_t* view) {
            auto ints = sugoi::get_owned<const IntComponent>(view);
            auto floats = sugoi::get_owned<const FloatComponent>(view);
            for (auto i = 0; i < view->count; i++)
            {
                EXPECT_EQ(ints[i].v, i);
                EXPECT_EQ(floats[i].v, i * 2.f);
            }

            // can get writable comps with readwrite query signature
            EXPECT_NE(sugoi::get_owned<IntComponent>(view), nullptr);
            EXPECT_NE(sugoi::get_owned<FloatComponent>(view), nullptr);
        };
        sugoiQ_get_views(q.value(), SUGOI_LAMBDA(callback));
    }
    // None
    {
        auto q = skr::ecs::QueryBuilder(&world)
                     .ReadAll<IntComponent>()
                     .None<FloatComponent>()
                     .commit();
        SKR_DEFER({ sugoiQ_release(q.value()); });
        EXPECT_OK(q);
        EXPECT_EQ(sugoiQ_get_count(q.value()), kIntEntityCount);
    }
    // WithMeta
    {
        auto q = skr::ecs::QueryBuilder(&world)
                     .ReadAll<IntComponent>()
                     .WithMetaEntity(shared_entity)
                     .commit();
        SKR_DEFER({ sugoiQ_release(q.value()); });
        EXPECT_OK(q);
        auto callback = [&](sugoi_chunk_view_t* view) {
            auto ints = sugoi::get_owned<const IntComponent>(view);
            auto floats = sugoi::get_owned<const FloatComponent>(view);
            EXPECT_NE(ints, nullptr);
            EXPECT_NE(floats, nullptr);

            sugoi_chunk_view_t shared_view = {};
            sugoiS_access(storage, (sugoi_entity_t)shared_entity, &shared_view);
            auto pshared = sugoi::get_owned<const SharedComponent>(&shared_view);
            EXPECT_EQ(pshared->i, 114);
            EXPECT_EQ(pshared->f, 514.f);
        };
        sugoiQ_get_views(q.value(), SUGOI_LAMBDA(callback));
        EXPECT_EQ(sugoiQ_get_count(q.value()), kIntEntityCount);
    }
    // WithoutMeta
    {
        auto q = skr::ecs::QueryBuilder(&world)
                     .ReadAll<IntComponent>()
                     .WithoutMetaEntity(shared_entity)
                     .commit();
        SKR_DEFER({ sugoiQ_release(q.value()); });
        EXPECT_OK(q);
        auto callback = [&](sugoi_chunk_view_t* view) {
            auto ints = sugoi::get_owned<const IntComponent>(view);
            auto floats = sugoi::get_owned<const FloatComponent>(view);
            EXPECT_NE(ints, nullptr);
            EXPECT_EQ(floats, nullptr);
        };
        sugoiQ_get_views(q.value(), SUGOI_LAMBDA(callback));
        EXPECT_EQ(sugoiQ_get_count(q.value()), kBothEntityCount);
    }
}

TEST_CASE_METHOD(AllocateEntites, "ParallelQueryCreate")
{
    SkrZoneScopedN("AllocateEntites::ParallelQueryCreate");

    skr::task::scheduler_t scheduler;
    scheduler.initialize(skr::task::scheudler_config_t());
    scheduler.bind();

    for (uint32_t i = 0; i < 128; i++)
    {
        skr::Vector<sugoi_query_t*> quries;
        quries.add_zeroed(i);
        skr::parallel_for(quries.data(), quries.data() + quries.size(), 1, [&](auto pbegin, auto pend) {
            const auto i = pbegin - quries.data();
            quries[i] = skr::ecs::QueryBuilder(&world)
                            .ReadAll<IntComponent>()
                            .commit()
                            .value();
            EXPECT_NE(quries[i], nullptr);
        });
        skr::parallel_for(quries.data(), quries.data() + quries.size(), 1, [&](auto pbegin, auto pend) {
            const auto i = pbegin - quries.data();
            sugoiQ_release(quries[i]);
        });
    }

    scheduler.unbind();
}
