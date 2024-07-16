#include "cpp_style.hpp"
#include "SkrTask/parallel_for.hpp"
#include "SkrRT/ecs/type_builder.hpp"
#include "SkrRT/ecs/storage.hpp"
#include "SkrRT/ecs/job.hpp"
#include "SkrCore/log.h"


struct ECSJobs {
    ECSJobs() SKR_NOEXCEPT
    {
        // std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        
        storage = sugoiS_create();

        spawnIntEntities();

        scheduler.initialize(skr::task::scheudler_config_t());
        scheduler.bind();
        sugoiJ_bind_storage(storage);
    }

    ~ECSJobs() SKR_NOEXCEPT
    {
        sugoiJ_unbind_storage(storage);
        ::sugoiS_release(storage);
        scheduler.unbind();

        // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    void spawnIntEntities()
    {
        SkrZoneScopedN("spawnIntEntities");
        sugoi::EntitySpawner<IntComponent, FloatComponent> spawner;
        spawner(storage, 2'000'000, 
            [&](auto& view){
                SkrZoneScopedN("memesetIntEntities");
                auto [ints, floats] = view.unpack();
                memset(ints, 0, sizeof(IntComponent) * view.count());
                memset(floats, 0, sizeof(FloatComponent) * view.count());
            });
    }

protected:
    sugoi_storage_t* storage = nullptr;
    skr::task::scheduler_t scheduler;
};

TEST_CASE_METHOD(ECSJobs, "WRW")
{
    SkrZoneScopedN("ECSJobs::WRW");

    auto ROQuery = storage->new_query()
            .ReadAll<IntComponent>()
            .commit().value();
    auto RWQuery = storage->new_query()
            .ReadWriteAll<IntComponent>()
            .commit().value();
    SKR_DEFER({ storage->destroy(ROQuery); storage->destroy(RWQuery); });

    auto WJob0 = SkrNewLambda(
        [=](sugoi_query_t* query, sugoi_chunk_view_t* view, sugoi_type_index_t* localTypes, EIndex entityIndex) {
        SkrZoneScopedN("WJob0");
        auto ints = sugoi::get_owned<IntComponent>(view);
        for (auto i = 0; i < view->count; i++)
        {
            EXPECT_EQ(ints[i].v, 0);
            ints[i].v = ints[i].v + 1;
        }
    });
    auto RJob = SkrNewLambda(
        [=](sugoi_query_t* query, sugoi_chunk_view_t* view, sugoi_type_index_t* localTypes, EIndex entityIndex) {
        SkrZoneScopedN("RJob");
        auto ints = sugoi::get_owned<const IntComponent>(view);
        for (auto i = 0; i < view->count; i++)
        {
            EXPECT_EQ(ints[i].v, 1);
        }
    });
    auto WJob1 = SkrNewLambda(
        [=](sugoi_query_t* query, sugoi_chunk_view_t* view, sugoi_type_index_t* localTypes, EIndex entityIndex) {
        SkrZoneScopedN("WJob1");
        auto ints = sugoi::get_owned<IntComponent>(view);
        for (auto i = 0; i < view->count; i++)
        {
            EXPECT_EQ(ints[i].v, 1);
            ints[i].v = ints[i].v + 1;
        }
    });
    auto& JS = sugoi::JobScheduler::Get();
    JS.schedule_ecs_job(RWQuery, 10'000, SUGOI_LAMBDA_POINTER(WJob0));
    JS.schedule_ecs_job(ROQuery, 10'000, SUGOI_LAMBDA_POINTER(RJob));
    JS.schedule_ecs_job(RWQuery, 10'000, SUGOI_LAMBDA_POINTER(WJob1));
    JS.sync_all_jobs();

    JS.collect_garbage();
}

TEST_CASE_METHOD(ECSJobs, "WRW-Complex")
{
    SkrZoneScopedN("ECSJobs::WRW-Complex");

    auto ROQuery = storage->new_query()
            .ReadAll<IntComponent>()
            .commit().value();
    auto RWIntQuery = storage->new_query()
            .ReadWriteAll<IntComponent>()
            .commit().value();
    auto RWFloatQuery = storage->new_query()
            .ReadWriteAll<FloatComponent>()
            .commit().value();
    SKR_DEFER({ 
        storage->destroy(ROQuery); 
        storage->destroy(RWIntQuery); 
        storage->destroy(RWFloatQuery); 
    });

    auto WriteInts = SkrNewLambda(
        [=](sugoi_query_t* query, sugoi_chunk_view_t* view, sugoi_type_index_t* localTypes, EIndex entityIndex) {
        SkrZoneScopedN("WriteInts");
        auto ints = sugoi::get_owned<IntComponent>(view);
        for (auto i = 0; i < view->count; i++)
        {
            ints[i].v = ints[i].v + 1;
        }
    });
    auto WriteFloats = SkrNewLambda(
        [=](sugoi_query_t* query, sugoi_chunk_view_t* view, sugoi_type_index_t* localTypes, EIndex entityIndex) {
        SkrZoneScopedN("WriteFloats");
        auto floats = sugoi::get_owned<FloatComponent>(view);
        for (auto i = 0; i < view->count; i++)
        {
            floats[i].v = floats[i].v + 1.f;
            floats[i].v = floats[i].v + 1.f;
        }
    });
    auto RJob = SkrNewLambda(
        [=](sugoi_query_t* query, sugoi_chunk_view_t* view, sugoi_type_index_t* localTypes, EIndex entityIndex) {
        SkrZoneScopedN("ReadOnlyJob");
        auto ints = sugoi::get_owned<const IntComponent>(view);
        // use an int-only query to schedule a job but reads floats
        // FloatComponents will not be synchronized but you can still read
        // which is an unsafe operation
        auto floats = sugoi::get_owned<const FloatComponent>(view);
        for (auto i = 0; i < view->count; i++)
        {
            EXPECT_EQ(ints[i].v, 1);
            const auto cond0 = floats[i].v == 0.f;
            const auto cond1 = floats[i].v == 1.f;
            const auto cond2 = floats[i].v == 2.f;
            const auto cond3 = floats[i].v == 3.f;
            const auto cond4 = floats[i].v == 4.f;
            const auto cond = cond0 || cond1 || cond2 || cond3 || cond4;
            EXPECT_TRUE(cond);
        }
    });
    auto& JS = sugoi::JobScheduler::Get();
    JS.schedule_ecs_job(RWIntQuery, 12'800, SUGOI_LAMBDA_POINTER(WriteInts));
    JS.schedule_ecs_job(RWFloatQuery, 2'560, SUGOI_LAMBDA_POINTER(WriteFloats));
    JS.schedule_ecs_job(RWFloatQuery, 2'560, SUGOI_LAMBDA_POINTER(WriteFloats));
    JS.schedule_ecs_job(ROQuery, 10'000, SUGOI_LAMBDA_POINTER(RJob));
    JS.sync_all_jobs();

    JS.collect_garbage();
}