#include "gtest/gtest.h"
#include "utils/make_zeroed.hpp"
#include "job/thread_job.hpp"

TEST(Job, JobQueue)
{
    auto jqDesc = make_zeroed<skr::JobQueueDesc>();
    jqDesc.thread_count = 2;
    jqDesc.priority = SKR_THREAD_NORMAL;
    auto jq = skr::JobQueue(&jqDesc);
    struct JI : public skr::JobItem
    {
        JI() : JobItem(u8"TestJob")
        {
            std::cout << "Ctor Test Job!" << std::endl;
        }
        skr::JobResult run() SKR_NOEXCEPT override
        {
            std::cout << "Hello Test Job!" << std::endl;
            return skr::JOB_RESULT_OK;
        }
        void finish(skr::JobResult result) SKR_NOEXCEPT override
        {
            std::cout << "Finish with result: " << result << std::endl;
        }
    } ji;
    jq.enqueue(&ji);
    jq.check();
    jq.wait_empty();
}

#include "utils/log.h"
#include "utils/async_task.hpp"
#include <containers/string.hpp>
#include <containers/sptr.hpp>

using Progress = int;
using InputParam1 = int;
using InputParam2 = int;
using Result = skr::string;
using Exception = skr::string;

struct AsyncFuture_ThreadJobQueue : public skr::IFuture<Result>
{
    skr::JobQueue* Q = nullptr;
    struct JBase : public skr::JobItem
    {
        JBase() : JobItem(u8"TestJob") {}
        void finish(skr::JobResult result) SKR_NOEXCEPT override
        {
            if (result == skr::JOB_RESULT_OK)
            {
                skr_atomic32_store_relaxed(&finished, true);
            }
        }
        skr::string result = "";
        SAtomic32 finished = false;
    };
    JBase* jobItem = nullptr;

    template<typename F, typename... Args>
    AsyncFuture_ThreadJobQueue(skr::JobQueue* Q, F&& _f, Args&&... args)
        : Q(Q)
    {
        struct JI : public JBase
        {
            skr::JobResult run() SKR_NOEXCEPT override { return runner(); }
            eastl::function<skr::JobResult()> runner;
        };
        JI* ji = SkrNew<JI>();
        ji->runner = [=](){ ji->result = _f(args...); return skr::JOB_RESULT_OK; };
        jobItem = ji;
        Q->enqueue(jobItem);
    }

    virtual bool valid() const SKR_NOEXCEPT { return true; }
    virtual void wait() SKR_NOEXCEPT
    {
        while (skr_atomic32_load_relaxed(&jobItem->finished) == false)
        {
            skr_thread_sleep(1);
        }
    }
    virtual skr::FutureStatus wait_for(uint32_t ms) SKR_NOEXCEPT
    {
        skr_thread_sleep(ms);
        const auto f = skr_atomic32_load_relaxed(&jobItem->finished);
        if (f) return skr::FutureStatus::Ready;
        return skr::FutureStatus::Timeout;
    }

    virtual Result get() SKR_NOEXCEPT { return jobItem->result; }
};

template<int TestIdx>
struct Launcher_ThreadJobQueue
{
    static skr::JobQueue* GetQueue()
    {
        static skr::SPtr<skr::JobQueue> jq = nullptr;
        if (!jq)
        {
            auto qn = skr::text::format(u8"Launcher{}_JobQueue", TestIdx);
            auto jqDesc = make_zeroed<skr::JobQueueDesc>();
            jqDesc.thread_count = 2;
            jqDesc.priority = SKR_THREAD_NORMAL;
            jqDesc.name = qn.u8_str();
            jq = skr::SPtr<skr::JobQueue>::Create(&jqDesc);
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
            Launcher_ThreadJobQueue::GetQueue(), std::forward<F>(f), std::forward<Args>(args)...);
    }
};

static const char* kCancelledResultString = "Empty, unfinished object";
static const char* kCompleteResultString = "Finished result object";

template<int TestIdx>
struct EmptyTaskWithProgressFeedback 
    : public skr::AsyncTask<Launcher_ThreadJobQueue<TestIdx>, Progress, Result, InputParam1, InputParam2>
{
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
        using Super = skr::AsyncTask<Launcher_ThreadJobQueue<TestIdx>, Progress, Result, InputParam1, InputParam2>;
        Launcher_ThreadJobQueue<TestIdx>::queue_update();
        return Super::on_callback_loop();
    }

    void on_pre_execute() override 
    { 
        SKR_LOG_DEBUG("Time-consuming calculation:\nProgress: 0%");
    }
    void on_progress_update(Progress const& progress) override 
    { 
        SKR_LOG_DEBUG("Time-consuming calculation:\nProgress: %d%", progress);
    }
    void on_post_execute(Result const& result) override 
    { 
        SKR_LOG_DEBUG("Progress is finished.");
    }
    void on_cancelled() override 
    {
        SKR_LOG_DEBUG("Progress is canceled.");
    }
    void on_exception(skr::AsyncTaskException* e) override
    {
        if (e)
        {
            SKR_LOG_DEBUG("AsyncTaskException: %d", (uint32_t)e->e);
        }
    }
};

TEST(Job, AsyncTask)
{
    EmptyTaskWithProgressFeedback<0> asynctask;
    asynctask.execute(1, 5);
    auto txt = skr::text::format(u8"{}", 1);
    for (int nRender = 0; !asynctask.on_callback_loop(); ++nRender) // if doInBackground() is finished it will stop the loop
    {
        skr_thread_sleep(120);

        if (nRender > 100) // -> Reduce this number to check Cancellation
            asynctask.cancel();
    }
    auto result = asynctask.get();
    SKR_LOG_DEBUG("Result: %s", result.c_str());
    EXPECT_EQ(result, kCompleteResultString);
}

TEST(Job, AsyncTaskCancel)
{
    EmptyTaskWithProgressFeedback<1> asynctask;
    asynctask.execute(100, 50);
    for (int nRender = 0; !asynctask.on_callback_loop(); ++nRender) // if doInBackground() is finished it will stop the loop
    {
        skr_thread_sleep(1);
        if (nRender > 20) // -> Reduce this number to check Cancellation
            asynctask.cancel();
    }
    auto result = asynctask.get();
    SKR_LOG_DEBUG("Result: %s", result.c_str());
    EXPECT_EQ(result, kCancelledResultString);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    return result;
}