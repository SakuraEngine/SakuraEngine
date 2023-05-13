#include "gtest/gtest.h"
#include "task/task2.hpp"



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
    using namespace skr::task2;
    int a = 0;
    event_t event;
    schedule([&]()
    {
        a = 10;
        event.notify();
    });
    sync(event);
    EXPECT_EQ(a, 10);
}

TEST_F(Task2, MultipleJob)
{
    using namespace skr::task2;
    int a = 0;
    int b = 0;
    event_t event;
    schedule([&]()
    {
        a = 10;
        event.notify();
    });
    event_t event2;

    schedule([&]()
    {
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
    using namespace skr::task2;
    int a = 0;
    event_t event;

    schedule([&]()
    {
        a = 10;
        event.notify();
    });
    event_t event2;
    schedule([&]() -> task_t
    {
        co_await wait(event);
        a += 10;
        event2.notify();
    }());
    sync(event2);
    EXPECT_EQ(a, 20);
}

TEST_F(Task2, NestedJob)
{
    using namespace skr::task2;
    int a = 0;
    event_t event;
    schedule([&]() -> task_t
    {
        a = 10;
        event_t event2;
        schedule([&]()
        {
            a += 10;
            event2.notify();
        });
        co_await wait(event2);
        a += 10;
        event.notify();
    }());
    sync(event);
    EXPECT_EQ(a, 30);
}


int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    return result;
}