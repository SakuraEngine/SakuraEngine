#include "gtest/gtest.h"

#if __cpp_impl_coroutine

#include "task/task2.hpp"
#include "platform/filesystem.hpp"
#include "module/module_manager.hpp"

class Task2 : public ::testing::Test
{
protected:
    skr::task2::scheduler_t scheduler;
    void SetUp() override
    {
        scheduler.initialize({});
        scheduler.bind();
    }

    void TearDown() override
    {
        scheduler.unbind();
        scheduler.shutdown();
    }
};


TEST_F(Task2, SingleJob)
{
    ZoneScopedN("SingleJob");
    using namespace skr::task2;
    int a = 0;
    event_t event;
    schedule([&]()
    {
        ZoneScopedN("Task");
        a = 10;
        event.notify();
    });
    sync(event);
    EXPECT_EQ(a, 10);
}

TEST_F(Task2, MultipleJob)
{
    ZoneScopedN("MultipleJob");
    using namespace skr::task2;
    int a = 0;
    int b = 0;
    event_t event;
    schedule([&]()
    {
        ZoneScopedN("Task1");
        a = 10;
        event.notify();
    });
    event_t event2;

    schedule([&]()
    {
        ZoneScopedN("Task2");
        b = 10;
        event2.notify();
    });

    sync(event);
    sync(event2);
    EXPECT_EQ(a, 10);
    EXPECT_EQ(b, 10);
}

TEST_F(Task2, JobWithDeps)
{
    ZoneScopedN("JobWithDeps");
    using namespace skr::task2;
    int a = 0;
    event_t event;

    schedule([&]()
    {
        ZoneScopedN("Task1");
        a = 10;
        event.notify();
    });
    event_t event2;
    schedule([](int& a, event_t event, event_t event2) mutable -> skr_task_t
    {
        TracyTask("Task2");
        ZoneScopedN("Task2");
        co_await co_wait(event);
        a += 10;
        event2.notify();
    }(a, event, event2));
    sync(event2);
    EXPECT_EQ(a, 20);
}

TEST_F(Task2, NestedJob)
{
    ZoneScopedN("NestedJob");
    using namespace skr::task2;
    int a = 0;
    event_t event;
    schedule([](int& a, event_t event) -> skr_task_t
    {
        TracyTask("Task1");
        ZoneScopedN("Task1");
        {
            ZoneScopedN("Task1-1");
            a = 10;
        }
        event_t event2;
        schedule([&]()
        {
            ZoneScopedN("Task2");
            a += 10;
            event2.notify();
        });
        co_await co_wait(event2);
        {
            ZoneScopedN("Task1-2");
            a += 10;
            event.notify();
        }
    }(a, event));
    sync(event);
    EXPECT_EQ(a, 30);
}

TEST_F(Task2, ParallelFor)
{
    ZoneScopedN("ParallelFor");
    using namespace skr::task2;
    std::atomic<int> a = 0;
    event_t event;
    auto coro = [](std::atomic<int>& a, event_t event, const char* name) -> skr_task_t
    {
        TracyTask(name);
        counter_t counter;
        counter.add(100);
        {
            ZoneScopedN("ScheduleLoop");
            for(int i=0; i<100; ++i)
            {
                schedule([=, &a]() mutable
                {
                    ZoneScopedN("LoopBody");
                    a += 10;
                    counter.decrease();
                });
            }
        }
        co_await co_wait(counter);
        a += 10;
        event.notify();
    };
    schedule(coro(a, event, "Outer"));
    sync(event);
    EXPECT_EQ(a, 1010);
}

TEST_F(Task2, ParallelForMassive)
{
    ZoneScopedN("ParallelForMassive");
    using namespace skr::task2;
    std::atomic<int> a = 0;
    counter_t event;
    event.add(10);
    auto coro = [](std::atomic<int>& a, counter_t event, const char* name) -> skr_task_t
    {
        TracyTask(name);
        counter_t counter;
        counter.add(1000);
        {
            ZoneScopedN("ScheduleLoop");
            for(int i=0; i<1000; ++i)
            {
                schedule([=, &a]() mutable
                {
                    a += 10;
                    counter.decrease();
                });
            }
        }
        co_await co_wait(counter);
        a += 10;
        event.decrease();
    };
    const char* names[] = {"Outer1", "Outer2", "Outer3", "Outer4", "Outer5", "Outer6", "Outer7", "Outer8", "Outer9", "Outer10"};
    for(int i=0; i<10; ++i)
    {
        schedule(coro(a, event, names[i]));
    }
    sync(event);
    EXPECT_EQ(a, 100100);
}

TEST_F(Task2, MassiveCoroutine)
{
    ZoneScopedN("MassiveCoroutine");
    using namespace skr::task2;
    std::atomic<int> a = 0;
    counter_t event;
    event.add(1000);
    auto coro = [](std::atomic<int>& a, counter_t event) -> skr_task_t
    {
        //ZoneScopedN("Coroutine");
        counter_t counter;
        SKR_ASSERT(counter);
        counter.add(100);
        {
            ZoneScopedN("ScheduleLoop");
            for(int i=0; i<100; ++i)
            {
                schedule([=, &a]() mutable
                {
                    a += 10;
                    counter.decrease();
                });
            }
        }
        SKR_ASSERT(counter);
        co_await co_wait(counter);
        a += 10;
        event.decrease();
    };
    for(int i=0; i<1000; ++i)
    {
        schedule(coro(a, event));
    }
    sync(event);
    EXPECT_EQ(a, 1010000);
}

int main(int argc, char** argv)
{
    auto moduleManager = skr_get_module_manager();
    std::error_code ec = {};
    auto root = skr::filesystem::current_path(ec);
    moduleManager->mount(root.u8string().c_str());
    moduleManager->make_module_graph(u8"Task2Test", true);
    moduleManager->init_module_graph(argc, argv);
    //while(!TracyIsConnected);
    ZoneScopedN("Main");
    ::testing::InitGoogleTest(&argc, argv);
    testing::FLAGS_gtest_repeat = 10;
    auto result = RUN_ALL_TESTS();
    moduleManager->destroy_module_graph();
    return result;
}

#else

int main(int argc, char** argv)
{
    return 0;
}

#endif