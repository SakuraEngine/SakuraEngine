#include "cpp_style.hpp"
#include "SkrTask/parallel_for.hpp"
#include "SkrRT/ecs/type_builder.hpp"
#include "SkrRT/ecs/storage.hpp"

struct ECSJobs {
    ECSJobs() SKR_NOEXCEPT
    {
        // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        
        storage = sugoiS_create();

        int_read_query = storage->new_query()
                .ReadAll<IntComponent>()
                .commit().value();

        int_write_query = storage->new_query()
                .ReadWriteAll<IntComponent>()
                .commit().value();

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
    sugoi_query_t* int_read_query = nullptr;
    sugoi_query_t* int_write_query = nullptr;
    sugoi_storage_t* storage = nullptr;
    skr::task::scheduler_t scheduler;
};

TEST_CASE_METHOD(ECSJobs, "WRW")
{
    SkrZoneScopedN("ECSJobs::WRW");

    auto WJob0 = SkrNewLambda([=](sugoi_query_t* query, sugoi_chunk_view_t* view, sugoi_type_index_t* localTypes, EIndex entityIndex) {
        SkrZoneScopedN("WJob0");
        auto ints = sugoi::get_owned<IntComponent>(view);
        for (auto i = 0; i < view->count; i++)
        {
            EXPECT_EQ(ints[i].v, 0);
            ints[i].v = ints[i].v + 1;
        }
    });
    auto RJob = SkrNewLambda([=](sugoi_query_t* query, sugoi_chunk_view_t* view, sugoi_type_index_t* localTypes, EIndex entityIndex) {
        SkrZoneScopedN("RJob");
        auto ints = sugoi::get_owned<const IntComponent>(view);
        for (auto i = 0; i < view->count; i++)
        {
            EXPECT_EQ(ints[i].v, 1);
        }
        
    });
    auto WJob1 = SkrNewLambda([=](sugoi_query_t* query, sugoi_chunk_view_t* view, sugoi_type_index_t* localTypes, EIndex entityIndex) {
        SkrZoneScopedN("WJob1");
        auto ints = sugoi::get_owned<IntComponent>(view);
        for (auto i = 0; i < view->count; i++)
        {
            EXPECT_EQ(ints[i].v, 1);
            ints[i].v = ints[i].v + 1;
        }
    });
    sugoiJ_schedule_ecs(int_write_query, 10'000, SUGOI_LAMBDA_POINTER(WJob0), nullptr, nullptr);
    sugoiJ_schedule_ecs(int_read_query, 10'000, SUGOI_LAMBDA_POINTER(RJob), nullptr, nullptr);
    sugoiJ_schedule_ecs(int_write_query, 10'000, SUGOI_LAMBDA_POINTER(WJob1), nullptr, nullptr);
}
