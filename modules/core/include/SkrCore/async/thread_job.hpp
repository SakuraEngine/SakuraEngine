#pragma once
#include "SkrContainers/string.hpp"
#include "SkrContainers/stl_list.hpp"
#include "SkrContainers/stl_vector.hpp"
#include "SkrContainers/stl_function.hpp"

#include "SkrCore/async/result.hpp"
#include "SkrCore/async/async_progress.hpp"

enum ESkrJobItemStatus
{
    kJobItemStatusNone = 0,
    kJobItemStatusWaiting,
    kJobItemStatusRunning,
    kJobItemStatusFinishJob
};

namespace skr
{
struct JobQueue;
struct JobThreadFunction;
struct JobThreadFunctionImpl;
struct JobQueueThread;
struct JobItemQueue;

using JobQueuePriority = SThreadPriority;
using JobResult = AsyncResult;
using JobName = skr::String;

struct JobQueueDesc
{
    const char8_t* name = nullptr;
    JobQueuePriority priority = SKR_THREAD_NORMAL;
    uint32_t stack_size = 16 * 1024;
    uint32_t thread_count = 1;
};

struct JobItemDesc
{
    int32_t __nothing__;
};

using EJobStatus = ESkrJobItemStatus;

struct SKR_STATIC_API JobItem
{
    friend struct JobQueue;
    friend struct JobThreadFunction;
    friend struct JobThreadFunctionImpl;
    friend struct JobItemQueue;

public:
    JobItem(const char8_t* name, const JobItemDesc& desc = {}) SKR_NOEXCEPT;
    JobItem(const JobItem& src) SKR_NOEXCEPT
        : status( skr_atomic32_load_acquire(&src.status) )
        , name(src.name), result(src.result), desc(src.desc)
    {

    }
    virtual ~JobItem() SKR_NOEXCEPT;

    // get job item name
    const char8_t* get_name() SKR_NOEXCEPT;

    // obtains JobItem's return code
    // @retval ASYNC_RESULT_OK if success
    JobResult get_result() SKR_NOEXCEPT;

	// returns if JobItem is executing
    // @retval true if JobItem is not executing
    bool is_none() SKR_NOEXCEPT;

protected:
    // virtual function to describe Job's sub thread processing.
    // @retval ASYNC_RESULT_OK if success
    virtual JobResult run() SKR_NOEXCEPT = 0;

    // vritual function to describe job's processing on termination
    virtual void finish(JobResult result) SKR_NOEXCEPT = 0;

    // vritual function to describe job's cancel
    // This function can be called, regardless of whether run() is running or not.
    // In any case, JobItem is responsible for working it correctly.
    // JobQueue don't call this function after starting finish().
    virtual void cancel() SKR_NOEXCEPT {}

private:
    SAtomic32 status;
    JobName name;
    int32_t result;
    JobItemDesc desc;
};

using JobQueueThreadList = skr::stl_list<JobQueueThread*>;

struct SKR_STATIC_API JobQueue
{
public:
    JobQueue(const JobQueueDesc& desc) SKR_NOEXCEPT;
	virtual ~JobQueue() SKR_NOEXCEPT;

    // enqueue JobItem object to job queue.
    // @retval ASYNC_RESULT_OK if success
	JobResult enqueue(JobItem* jobItem) SKR_NOEXCEPT;

    // get the number of JobItem objects enqueued in queue.
	uint32_t items_count() const SKR_NOEXCEPT;

    // wait until job queue gets empty, this function needs to be called from sub thread. 
    // if you call this function from main thread or from run() of a Job queued in JobQueue, deadlock will occur.
    void wait_empty() const SKR_NOEXCEPT;

    // change thread priority
    void change_priority(JobQueuePriority priority) SKR_NOEXCEPT;

    // return a boolean value that indicates there is JobItems or not queued in JobQueue.
    // if it is true, then it is enable to delete JobItems.
    bool is_empty() SKR_NOEXCEPT;

    // check JobItems queued in JobQueue
    // this function needs to be called once from main thread every frame.
    // @retval ASYNC_RESULT_OK if success
	JobResult check() SKR_NOEXCEPT;

    // returns descriptor specified to JobQueue initialize
    void get_descriptor(JobQueueDesc* desc) SKR_NOEXCEPT;

    // cancel all enqueued jobs. 
    // @retval ASYNC_RESULT_OK if success
    JobResult cancel_all_items() SKR_NOEXCEPT;

private:
    friend struct JobThreadFunction;
    int enqueueCore(JobItem* jobItem, bool isEndJob) SKR_NOEXCEPT;

    // initialize JobQueue
    // @retval ASYNC_RESULT_OK if success
	JobResult initialize() SKR_NOEXCEPT;

    // finalize JobQueue.
    // @retval ASYNC_RESULT_OK if success
	JobResult finalize() SKR_NOEXCEPT;

    skr::String queue_name;
    JobQueueThreadList thread_list;
    JobItemQueue* itemList;
    JobQueueDesc desc;

    skr::stl_vector<JobItem*> pending_queue;
    SRWMutex pending_queue_mutex;
    SAtomic32 cancel_requested = 0;
};

struct SKR_STATIC_API ThreadedJobQueueFutureJob : public skr::JobItem
{
    ThreadedJobQueueFutureJob(JobQueue* Q) SKR_NOEXCEPT;
    virtual ~ThreadedJobQueueFutureJob() SKR_NOEXCEPT;
    void finish(skr::JobResult result) SKR_NOEXCEPT override;

    // implementations for future<> to call.
    
    bool valid() const SKR_NOEXCEPT;
    void wait() SKR_NOEXCEPT;
    skr::FutureStatus wait_for(uint32_t ms) SKR_NOEXCEPT;

    // end implementations for future<> to call.

    SAtomic32 finished = false;
    skr::JobQueue* Q = nullptr;
};

template<typename Artifact, typename JobBaseType = ThreadedJobQueueFutureJob>
struct ThreadedJobQueueFuture : public skr::IFuture<Artifact>
{
    template<typename F, typename... Args>
    ThreadedJobQueueFuture(skr::JobQueue* Q, F&& _f, Args&&... args)
        : job(Q)
    {
        job.runner = [=, This = this]() { 
            This->artifact = _f(args...); 
            This->job.finish(skr::ASYNC_RESULT_OK);
            return skr::ASYNC_RESULT_OK; 
        };
        Q->enqueue(&job);
    }
    virtual ~ThreadedJobQueueFuture() SKR_NOEXCEPT {}

    Artifact get() SKR_NOEXCEPT override { return artifact; }
    bool valid() const SKR_NOEXCEPT override { return job.valid(); }
    void wait() SKR_NOEXCEPT override { job.wait(); }
    skr::FutureStatus wait_for(uint32_t ms) SKR_NOEXCEPT override { return job.wait_for(ms); }

    struct JobType : public JobBaseType
    {
        JobType(JobQueue* Q) : JobBaseType(Q) {}
        skr::JobResult run() SKR_NOEXCEPT override 
        { 
            // BaseType::run is not called currently.
            auto ret = runner();
            return ret; 
        }
        skr::stl_function<skr::JobResult()> runner;
    };

protected:
    JobType job;
    Artifact artifact;
};

template<typename R>
struct FutureLauncher
{
    using JobQueueFuture = ThreadedJobQueueFuture<R>;
    using SerialFuture = skr::SerialFuture<R>;

    FutureLauncher(skr::JobQueue* q) : job_queue(q) {}
    template<typename F, typename... Args>
    skr::IFuture<R>* async(F&& f, Args&&... args)
    {
        if (job_queue)
            return SkrNew<JobQueueFuture>(job_queue, std::forward<F>(f), std::forward<Args>(args)...);
        else
            return SkrNew<SerialFuture>(std::forward<F>(f), std::forward<Args>(args)...);
    }
    skr::JobQueue* job_queue = nullptr;
};

}