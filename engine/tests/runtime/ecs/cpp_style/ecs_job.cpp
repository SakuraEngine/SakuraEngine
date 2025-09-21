#include "cpp_style.hpp"
#include "SkrTask/parallel_for.hpp"
#include "SkrRuntime/ecs/world.hpp"
#include "SkrCore/log.h"

#define TEST_ENTITY_COUNT 2'000'000

struct ECSJobs {
    ECSJobs() SKR_NOEXCEPT
        : world(scheduler)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1200));
        scheduler.initialize(skr::task::scheudler_config_t());
        scheduler.bind();
     
        world.initialize();

        spawnIntEntities();
    }

    ~ECSJobs() SKR_NOEXCEPT
    {
        world.finalize();
        scheduler.unbind();
         std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }

    void spawnIntEntities()
    {
        SkrZoneScopedN("spawnIntEntities");
        struct Spawner
        {
            void build(skr::ecs::ArchetypeBuilder& Builder)
            {
                Builder.add_component(&Spawner::ints)
                       .add_component(&Spawner::floats);
            }

            void run(skr::ecs::TaskContext& Context)
            {
                memset(&ints[0], 0, sizeof(IntComponent) * Context.size());
                memset(&floats[0], 0, sizeof(FloatComponent) * Context.size());
            }

            skr::ecs::ComponentView<IntComponent> ints;
            skr::ecs::ComponentView<FloatComponent> floats;
        } spawner;
        world.create_entities(spawner, TEST_ENTITY_COUNT);
    }

protected:
    skr::ecs::ECSWorld world;
    skr::task::scheduler_t scheduler;
};

TEST_CASE_METHOD(ECSJobs, "WRW")
{
    SkrZoneScopedN("ECSJobs::WRW");

    skr::ServiceThreadDesc desc = {
        .name = u8"ECSJob-TaskScheduler",
        .priority = SKR_THREAD_ABOVE_NORMAL
    };

    struct WriteJob0
    {
        void build(skr::ecs::AccessBuilder& Builder)
        {
            Builder.write(&WriteJob0::ints);
        }
        void run(skr::ecs::TaskContext& Context)
        {
            SkrZoneScopedN("WriteJob0");
            for (auto i = 0; i < Context.size(); i++)
            {
                EXPECT_EQ(ints[i].v, 0);
                ints[i].v = ints[i].v + 1;
            }
        }
        skr::ecs::ComponentView<IntComponent> ints;
    } wjob0;
    auto q0 = world.dispatch_task(wjob0, 1'280, nullptr);

    struct ReadJob
    {
        void build(skr::ecs::AccessBuilder& Builder)
        {
            Builder.read(&ReadJob::ints);
        }
        void run(skr::ecs::TaskContext& Context)
        {
            SkrZoneScopedN("ReadJob");
            for (auto i = 0; i < Context.size(); i++)
            {
                EXPECT_EQ(ints[i].v, 1);
            }
        }
        skr::ecs::ComponentView<const IntComponent> ints;
    } rjob;
    auto q1 = world.dispatch_task(rjob, 1'280, nullptr);

    struct WriteJob1
    {
        void build(skr::ecs::AccessBuilder& Builder)
        {
            Builder.write(&WriteJob1::ints);
        }
        void run(skr::ecs::TaskContext& Context)
        {
            SkrZoneScopedN("WriteJob1");
            for (auto i = 0; i < Context.size(); i++)
            {
                EXPECT_EQ(ints[i].v, 1);
                ints[i].v = ints[i].v + 1;
            }
        }
        skr::ecs::ComponentView<IntComponent> ints;
    } wjob1;
    world.dispatch_task(wjob1, 1'280, q0);

    skr::ecs::TaskScheduler::Get()->flush_all();
    skr::ecs::TaskScheduler::Get()->sync_all();

    world.destroy_query(q0);
    world.destroy_query(q1);
}

TEST_CASE_METHOD(ECSJobs, "WRW-Complex")
{
    SkrZoneScopedN("ECSJobs::WRW-Complex");

    struct WriteInts
    {
        void build(skr::ecs::AccessBuilder& Builder)
        {
            Builder.write(&WriteInts::ints);
        }
        void run(skr::ecs::TaskContext& Context)
        {
            SkrZoneScopedN("WriteInts");
            for (auto i = 0; i < Context.size(); i++)
            {
                ints[i].v = ints[i].v + 1;
            }
        }
        skr::ecs::ComponentView<IntComponent> ints;
    } writeInts;
    auto q0 = world.dispatch_task(writeInts, 1'280, nullptr);

    struct WriteFloats
    {
        void build(skr::ecs::AccessBuilder& Builder)
        {
            Builder.write(&WriteFloats::floats);
        }
        void run(skr::ecs::TaskContext& Context)
        {
            SkrZoneScopedN("WriteFloats");
            for (auto i = 0; i < Context.size(); i++)
            {
                floats[i].v = floats[i].v + 1.f;
                floats[i].v = floats[i].v + 1.f;
            }
        }
        skr::ecs::ComponentView<FloatComponent> floats;
    } writeFloats;
    auto q1 = world.dispatch_task(writeFloats, 1'280, nullptr);

    struct ReadJob
    {
        void build(skr::ecs::AccessBuilder& Builder)
        {
            Builder.read(&ReadJob::ints)
                .read(&ReadJob::floats);
        }
        void run(skr::ecs::TaskContext& Context)
        {
            SkrZoneScopedN("ReadOnlyJob");
            for (auto i = 0; i < Context.size(); i++)
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
        }
        skr::ecs::ComponentView<const IntComponent> ints;
        skr::ecs::ComponentView<const FloatComponent> floats;
    } readJob;
    auto q2 = world.dispatch_task(readJob, 1'280, nullptr);

    skr::ecs::TaskScheduler::Get()->flush_all();
    skr::ecs::TaskScheduler::Get()->sync_all();

    world.destroy_query(q0);
    world.destroy_query(q1);
    world.destroy_query(q2);
}

TEST_CASE_METHOD(ECSJobs, "RandomAccess")
{
    static std::atomic_uint64_t cnt = 0;
    static std::atomic<skr::ecs::Entity> some_entity;
    static std::atomic_uint64_t some_value = 0;

    struct WriteInts
    {
        void build(skr::ecs::AccessBuilder& Builder)
        {
            Builder.write(&WriteInts::ints);
        }
        void run(skr::ecs::TaskContext& Context)
        {
            SkrZoneScopedN("WriteInts");
            for (auto i = 0; i < Context.size(); i++)
            {
                ints[i].v = ints[i].v + 1;
                cnt += 1;
            }

            some_entity = Context.entities()[0];
            some_value = ints[0].v;
        }
        skr::ecs::ComponentView<IntComponent> ints;
    } writeInts;
    auto q0 = world.dispatch_task(writeInts, 1'280, nullptr);

    static std::atomic_uint64_t parallel_read_cnt = 0;
    static std::atomic_uint64_t max_parallel_read_cnt = 0;
    struct RandomReadInts
    {
        void build(skr::ecs::AccessBuilder& Builder)
        {
            Builder.read(&RandomReadInts::ints) 
                .access(&RandomReadInts::int_accessor);
        }
        void run(skr::ecs::TaskContext& Context)
        {
            SkrZoneScopedN("RandomReadInts");
            parallel_read_cnt += 1;
            max_parallel_read_cnt = std::max(max_parallel_read_cnt.load(), parallel_read_cnt.load());

            EXPECT_EQ(cnt, TEST_ENTITY_COUNT);
            
            for (auto i = 0; i < Context.size(); i++)
            {
                EXPECT_EQ(ints[i].v, 1);
            }

            IntComponent value = int_accessor[some_entity];
            EXPECT_EQ(value.v, some_value);
            
            parallel_read_cnt -= 1;
        }

        skr::ecs::ComponentView<const IntComponent> ints;
        skr::ecs::RandomComponentReader<const IntComponent> int_accessor;
    } randomReadInts;
    auto q1 = world.dispatch_task(randomReadInts, 1'280, nullptr);

    static std::atomic_uint64_t parallel_write_cnt = 0;
    static std::atomic_uint64_t max_parallel_write_cnt = 0;
    struct RandomWriteInts
    {
        void build(skr::ecs::AccessBuilder& Builder)
        {
            Builder.write(&RandomWriteInts::floats)
                .access(&RandomWriteInts::int_accessor);
        }
        void run(skr::ecs::TaskContext& Context)
        {
            SkrZoneScopedN("RandomWriteInts");
            parallel_write_cnt += 1;
            max_parallel_write_cnt = std::max(max_parallel_write_cnt.load(), parallel_write_cnt.load());

            EXPECT_EQ(cnt, TEST_ENTITY_COUNT);

            for (auto i = 0; i < Context.size(); i++)
            {
                EXPECT_EQ(floats[i].v, 0);
            }
            int_accessor.write_at(some_entity, IntComponent{5});

            parallel_write_cnt -= 1;
        }
        skr::ecs::ComponentView<FloatComponent> floats;
        skr::ecs::RandomComponentWriter<IntComponent> int_accessor;
    } randomWriteInts;
    auto q2 = world.dispatch_task(randomWriteInts, 1'280, nullptr);


    static std::atomic_uint64_t parallel_rw_cnt = 0;
    static std::atomic_uint64_t max_parallel_rw_cnt = 0;
    struct RandomReadWriteInts
    {
        void build(skr::ecs::AccessBuilder& Builder)
        {
            Builder.access(&RandomReadWriteInts::int_accessor);
        }
        void run(skr::ecs::TaskContext& Context)
        {
            SkrZoneScopedN("RandomReadWriteInts");
            parallel_rw_cnt += 1;
            max_parallel_rw_cnt = std::max(max_parallel_rw_cnt.load(), parallel_rw_cnt.load());

            EXPECT_EQ(cnt, TEST_ENTITY_COUNT);

            IntComponent value = int_accessor[some_entity];
            int_accessor.write_at(some_entity, IntComponent{10});

            parallel_rw_cnt -= 1;
        }
        skr::ecs::RandomComponentReadWrite<IntComponent> int_accessor;
    } randomReadWriteInts;
    auto q3 = world.dispatch_task(randomReadWriteInts, 1'280, nullptr);

    skr::ecs::TaskScheduler::Get()->flush_all();
    skr::ecs::TaskScheduler::Get()->sync_all();

    EXPECT_NE(max_parallel_read_cnt, 1);
    EXPECT_NE(max_parallel_write_cnt, 1);
    EXPECT_EQ(max_parallel_rw_cnt, 1);

    world.destroy_query(q0);
    world.destroy_query(q1);
    world.destroy_query(q2);
    world.destroy_query(q3);
}