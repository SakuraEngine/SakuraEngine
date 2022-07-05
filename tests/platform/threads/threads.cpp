#include "gtest/gtest.h"
#include "platform/thread.h"
#include <thread>
#include <future>

class Threads : public ::testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST(Threads, CallOnce)
{
    SCallOnceGuard guard;
    skr_init_call_once_guard(&guard);
    static std::atomic_uint counter = 0;
    auto onceF = [&]() {
        skr_call_once(&guard, []() {
            EXPECT_EQ(1, 1);
            counter++;
            std::cout << "CallOnce!" << std::endl;
        });
    };
    std::thread st1(onceF);
    std::thread st2(onceF);
    std::thread st3(onceF);
    std::thread st4(onceF);
    st1.join();
    st2.join();
    st3.join();
    st4.join();
    EXPECT_EQ(counter, 1);
}

TEST(Threads, CondVar)
{
    SConditionVariable cv;
    SMutex sm;
    skr_init_condition_var(&cv);
    skr_init_mutex(&sm);
    std::cout << "Hello0" << std::endl;
    auto future = std::async([&]() {
        skr_wait_condition_vars(&cv, &sm, UINT32_MAX);
        std::cout << "Hello2" << std::endl;
    });
    skr_thread_sleep(1000);
    std::cout << "Hello1" << std::endl;
    skr_wake_condition_var(&cv);
    future.wait();
    skr_destroy_condition_var(&cv);
    skr_destroy_mutex(&sm);
}

#include "platform/atomic.h"

TEST(Threads, Atomic)
{
    SAtomic32 a32 = 0;
    auto addF = [&]() {
        skr_atomic32_add_relaxed(&a32, 1);
    };
    std::thread st1(addF);
    std::thread st2(addF);
    std::thread st3(addF);
    std::thread st4(addF);
    st1.join();
    st2.join();
    st3.join();
    st4.join();
    EXPECT_EQ(a32, 4);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    return result;
}