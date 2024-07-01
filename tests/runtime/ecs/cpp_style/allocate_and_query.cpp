#include "cpp_style.hpp"
#include "SkrRT/ecs/type_builder.hpp"
#include "SkrRT/ecs/storage.hpp"

struct AllocateEntites {
    AllocateEntites() SKR_NOEXCEPT
    {
        storage = sugoiS_create();
    }
    ~AllocateEntites() SKR_NOEXCEPT
    {
        ::sugoiS_release(storage);
    }
    sugoi_storage_t* storage = nullptr;
};

static constexpr size_t kIntEntityCount = 12;
static constexpr size_t kFloatEntityCount = 12;
static constexpr size_t kBothEntityCount = 12;
sugoi_entity_t shared_entity = ~0;

TEST_CASE_METHOD(AllocateEntites, "AllocateAndQuery")
{
    {
        sugoi::EntitySpawner<SharedComponent> spawner;
        spawner(storage, 1, 
        [&](auto& view){
            auto pcomp = sugoi::get_owned<SharedComponent>(view.view);
            pcomp[0].i = 114;
            pcomp[0].f = 514.f;
            EXPECT_EQ(view.count(), 1);
            shared_entity = sugoiV_get_entities(view.view)[0];
        });
    }
    {
        sugoi::EntitySpawner<IntComponent> spawner;
        spawner(storage, kIntEntityCount, 
            [&](auto& view){
                auto [ints] = view.unpack();
                for (auto i = 0; i < view.count(); i++)
                {
                    ints[i].v = i;
                }
                EXPECT_EQ(view.count(), kIntEntityCount);
            });
    }
    {
        sugoi::EntitySpawner<FloatComponent> spawner;
        spawner(storage, kFloatEntityCount, 
            [&](auto& view){
                auto [floats] = view.unpack();
                for (auto i = 0; i < view.count(); i++)
                {
                    floats[i].v = i * 2.f;
                }
                EXPECT_EQ(view.count(), kFloatEntityCount);
            });
    }
    {
        sugoi::EntitySpawner<IntComponent, FloatComponent> spawner(shared_entity);
        spawner(storage, kBothEntityCount, 
            [&](auto& view){
                auto [ints, floats] = view.unpack();
                for (auto i = 0; i < view.count(); i++)
                {
                    ints[i].v = i;
                    floats[i].v = i * 2.f;
                }
                EXPECT_EQ(view.count(), kBothEntityCount);
            });
    }
    // ReadAll
    {
        auto q = storage->new_query()
                    .ReadAll<FloatComponent, IntComponent>()
                    .commit();
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
        auto q = storage->new_query()
                .ReadWriteAll<FloatComponent, IntComponent>()
                .commit();
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
        auto q = storage->new_query()
                    .ReadAll<IntComponent>()
                    .None<FloatComponent>()
                    .commit();
        EXPECT_OK(q);
        EXPECT_EQ(sugoiQ_get_count(q.value()), kIntEntityCount);
    }
    // WithMeta
    {
        auto q = storage->new_query()
                    .ReadAll<IntComponent>()
                    .WithMetaEntity(shared_entity)
                    .commit();
        EXPECT_OK(q);
        auto callback = [&](sugoi_chunk_view_t* view) {
            auto ints = sugoi::get_owned<const IntComponent>(view);
            auto floats = sugoi::get_owned<const FloatComponent>(view);
            EXPECT_NE(ints, nullptr);       
            EXPECT_NE(floats, nullptr);       

            sugoi_chunk_view_t shared_view = {};
            sugoiS_access(storage, shared_entity, &shared_view);
            auto pshared = sugoi::get_owned<const SharedComponent>(&shared_view);
            EXPECT_EQ(pshared->i, 114);
            EXPECT_EQ(pshared->f, 514.f);
        };
        sugoiQ_get_views(q.value(), SUGOI_LAMBDA(callback));
        EXPECT_EQ(sugoiQ_get_count(q.value()), kIntEntityCount);
    }
    // WithoutMeta
    {
        auto q = storage->new_query()
                    .ReadAll<IntComponent>()
                    .WithoutMetaEntity(shared_entity)
                    .commit();
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