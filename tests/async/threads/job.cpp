#include "SkrCore/crash.h"
#include "SkrCore/log.h"
#include "SkrBase/misc/make_zeroed.hpp"
#include "SkrCore/async/thread_job.hpp"

#include "SkrCore/memory/sp.hpp"
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

TEST_CASE("JobQueue")
{
    auto jqDesc = make_zeroed<skr::JobQueueDesc>();
    jqDesc.thread_count = 2;
    jqDesc.priority = SKR_THREAD_NORMAL;
    auto jq = skr::JobQueue(jqDesc);
    struct JI : public skr::JobItem
    {
        JI() : JobItem(u8"TestJob")
        {
            SKR_TEST_INFO(u8"Ctor Test Job!");
        }
        skr::JobResult run() SKR_NOEXCEPT override
        {
            SKR_TEST_INFO(u8"Hello Test Job!");
            return skr::ASYNC_RESULT_OK;
        }
        void finish(skr::JobResult result) SKR_NOEXCEPT override
        {
            SKR_TEST_INFO(u8"Finish with result: {}", result);
        }
    } ji;
    jq.enqueue(&ji);
    jq.check();
    jq.wait_empty();
}

#include "SkrCore/log.h"
#include "SkrCore/async/async_progress.hpp"
#include <SkrContainers/string.hpp>

using Progress = int;
using InputParam1 = int;
using InputParam2 = int;
using Result = skr::String;
using Exception = skr::String;

struct AsyncFuture_ThreadJobQueue : public skr::IFuture<Result>
{
    skr::JobQueue* Q = nullptr;
    struct JBase : public skr::JobItem
    {
        virtual ~JBase() { SKR_TEST_INFO(u8"Job Base dtor"); }
        JBase() : JobItem(u8"TestJob") {}
        void finish(skr::JobResult result) SKR_NOEXCEPT override
        {
            if (result == skr::ASYNC_RESULT_OK)
            {
                skr_atomic_store_relaxed(&finished, true);
            }
        }
        skr::String result = u8"";
        SAtomic32 finished = false;
    };
    JBase* jobItem = nullptr;

    template<typename F, typename... Args>
    AsyncFuture_ThreadJobQueue(skr::JobQueue* Q, F&& _f, Args&&... args)
        : Q(Q)
    {
        struct JI : public JBase
        {
            ~JI() override { SKR_TEST_INFO(u8"Job Instance dtor"); }
            skr::JobResult run() SKR_NOEXCEPT override { return runner(); }
            skr::stl_function<skr::JobResult()> runner;
        };
        JI* ji = SkrNew<JI>();
        ji->runner = [=](){ ji->result = _f(args...); return skr::ASYNC_RESULT_OK; };
        jobItem = ji;
        Q->enqueue(jobItem);
    }

    virtual ~AsyncFuture_ThreadJobQueue() { SkrDelete(jobItem); SKR_TEST_INFO(u8"Future dtor"); }

    virtual bool valid() const SKR_NOEXCEPT { return true; }
    virtual void wait() SKR_NOEXCEPT
    {
        while (skr_atomic_load_relaxed(&jobItem->finished) == false)
        {
            skr_thread_sleep(1);
        }
    }
    virtual skr::FutureStatus wait_for(uint32_t ms) SKR_NOEXCEPT
    {
        skr_thread_sleep(ms);
        const auto f = skr_atomic_load_relaxed(&jobItem->finished);
        if (f) return skr::FutureStatus::Ready;
        return skr::FutureStatus::Timeout;
    }

    virtual Result get() SKR_NOEXCEPT { return jobItem->result; }
};

template<int TestIdx>
struct Launcher_ThreadJobQueue
{
    ~Launcher_ThreadJobQueue()
    {
        SKR_TEST_INFO(u8"Launcher dtor");
    }
    static skr::JobQueue* GetQueue()
    {
        static skr::SP<skr::JobQueue> jq = nullptr;
        if (!jq)
        {
            auto qn = skr::format(u8"Launcher{}_JobQueue", TestIdx);
            auto jqDesc = make_zeroed<skr::JobQueueDesc>();
            jqDesc.thread_count = 2;
            jqDesc.priority = SKR_THREAD_NORMAL;
            jqDesc.name = qn.c_str();
            jq = skr::SP<skr::JobQueue>::New(jqDesc);
        }
        SKR_ASSERT(jq);
        return jq.get();
    }

    static void queue_update()
    {
        GetQueue()->check();
    }

    template<typename F, typename... Args>
    static AsyncFuture_ThreadJobQueue* async(F&& f, Args&&... args)
    {
        return SkrNew<AsyncFuture_ThreadJobQueue>(
            Launcher_ThreadJobQueue::GetQueue(), 
            std::forward<F>(f), std::forward<Args>(args)...
        );
    }
};

static const char8_t* kCancelledResultString = u8"Empty, unfinished object";
static const char8_t* kCompleteResultString = u8"Finished result object";

template<int TestIdx>
struct EmptyTaskWithProgressFeedback 
    : public skr::AsyncProgress<Launcher_ThreadJobQueue<TestIdx>, Progress, Result, InputParam1, InputParam2>
{
    ~EmptyTaskWithProgressFeedback() override
    {
        SKR_TEST_INFO(u8"EmptyTaskWithProgressFeedback dtor");
    }

    Result do_in_background(InputParam1 const& p1, InputParam2 const& p2) override
    {
        auto const n = p1 + p2;
        for (int i = 0; i <= n; ++i)
        {
            skr_thread_sleep(50);

            this->publish_progress(i);

            if (this->is_cancelled()) return kCancelledResultString;
        }
        return kCompleteResultString;
    }

    bool on_callback_loop() override
    {
        using Super = skr::AsyncProgress<Launcher_ThreadJobQueue<TestIdx>, Progress, Result, InputParam1, InputParam2>;
        Launcher_ThreadJobQueue<TestIdx>::queue_update();
        return Super::on_callback_loop();
    }

    void on_pre_execute() override 
    { 
        SKR_TEST_INFO(u8"Time-consuming calculation:\nProgress: 0%");
    }
    void on_progress_update(Progress const& progress) override 
    { 
        SKR_TEST_INFO(u8"Time-consuming calculation:\nProgress: {}", progress);
    }
    void on_post_execute(Result const& result) override 
    { 
        SKR_TEST_INFO(u8"Progress is finished.");
    }
    void on_cancelled() override 
    {
        SKR_TEST_INFO(u8"Progress is canceled.");
    }
    void on_exception(skr::AsyncProgressException* e) override
    {
        if (e)
        {
            SKR_TEST_INFO(u8"AsyncProgressException: %d", (uint32_t)e->e);
        }
    }
};

TEST_CASE("AsyncProgress")
{
    EmptyTaskWithProgressFeedback<0> AsyncProgress;
    AsyncProgress.execute(1, 5);
    auto txt = skr::format(u8"{}", 1);
    for (int nRender = 0; !AsyncProgress.on_callback_loop(); ++nRender) // if doInBackground() is finished it will stop the loop
    {
        skr_thread_sleep(120);

        if (nRender > 100) // -> Reduce this number to check Cancellation
            AsyncProgress.cancel();
    }
    auto result = AsyncProgress.get_result();
    SKR_TEST_INFO(u8"Result: {}", result.c_str());
    EXPECT_EQ(result, kCompleteResultString);
}

TEST_CASE("AsyncProgressCancel")
{
    EmptyTaskWithProgressFeedback<1> AsyncProgress;
    AsyncProgress.execute(100, 50);
    for (int nRender = 0; !AsyncProgress.on_callback_loop(); ++nRender) // if doInBackground() is finished it will stop the loop
    {
        skr_thread_sleep(1);
        if (nRender > 20) // -> Reduce this number to check Cancellation
            AsyncProgress.cancel();
    }
    auto result = AsyncProgress.get_result();
    SKR_TEST_INFO(u8"Result: {}", result.c_str());
    EXPECT_EQ(result, kCancelledResultString);
}