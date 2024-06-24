#include "SkrOS/thread.h"
#include "SkrCore/crash.h"
#include "SkrCore/log.h"
#include <thread>
#include <future>

#include "SkrTestFramework/framework.hpp"

static struct ProcInitializer
{
    ProcInitializer()
    {
        ::skr_log_set_level(SKR_LOG_LEVEL_WARN);
        // ::skr_initialize_crash_handler();
        ::skr_log_initialize_async_worker();
    }
    ~ProcInitializer()
    {
        ::skr_log_finalize_async_worker();
        // ::skr_finalize_crash_handler();
    }
} init;

class ThreadsTest
{
protected:
    ThreadsTest()
    {

    }
    ~ThreadsTest()
    {

    }
};

TEST_CASE_METHOD(ThreadsTest, "CallOnce")
{
    SCallOnceGuard guard;
    skr_init_call_once_guard(&guard);
    static std::atomic_uint counter = 0;
    auto onceF = [&]() {
        skr_call_once(&guard, []() {
            EXPECT_EQ(1, 1);
            counter++;
            SKR_TEST_INFO(u8"CallOnce!");
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

TEST_CASE_METHOD(ThreadsTest, "CondVar")
{
    SConditionVariable cv;
    SMutex sm;
    skr_init_condition_var(&cv);
    skr_init_mutex(&sm);
    SKR_TEST_INFO(u8"Hello0!");
    auto future = std::async([&]() {
        SMutexLock lock(sm);
        skr_wait_condition_vars(&cv, &sm, UINT32_MAX);
        SKR_TEST_INFO(u8"Hello2!");
    });
    skr_thread_sleep(1000);
    SKR_TEST_INFO(u8"Hello1!");
    {
        SMutexLock lock(sm);
        skr_wake_condition_var(&cv);
    }
    future.wait();
    skr_destroy_condition_var(&cv);
    skr_destroy_mutex(&sm);
}

TEST_CASE_METHOD(ThreadsTest, "RecursiveCondVar")
{
    SConditionVariable cv;
    SMutex sm;
    skr_init_condition_var(&cv);
    skr_init_mutex_recursive(&sm);
    SKR_TEST_INFO(u8"Hello0!");
    auto future = std::async([&]() {
        SMutexLock lock(sm);
        skr_wait_condition_vars(&cv, &sm, UINT32_MAX);
        SKR_TEST_INFO(u8"Hello2!");
    });
    skr_thread_sleep(1000);
    SKR_TEST_INFO(u8"Hello1!");
    {
        SMutexLock lock(sm);
        skr_wake_condition_var(&cv);
    }
    future.wait();
    skr_destroy_condition_var(&cv);
    skr_destroy_mutex(&sm);
}

#include "SkrBase/atomic/atomic.h"

TEST_CASE_METHOD(ThreadsTest, "Atomic")
{
    SAtomicU32 a32 = 0;
    auto addF = [&]() {
        skr_atomic_fetch_add_relaxed(&a32, 1);
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