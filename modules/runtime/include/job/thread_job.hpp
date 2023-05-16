#pragma once
#include "platform/configure.h"
#include "platform/thread.h"
#include "platform/atomic.h"
#include "containers/text.hpp"
#include "containers/vector.hpp"
#include <EASTL/list.h>

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
using JobResult = int32_t;
using JobName = skr::text::text;

static constexpr JobResult JOB_RESULT_OK = 1;
static constexpr JobResult JOB_RESULT_ERROR_THREAD_ALREADY_STARTES = -1;
static constexpr JobResult JOB_RESULT_ERROR_COND_CREATE_FAILED = -2;
static constexpr JobResult JOB_RESULT_ERROR_COND_MX_CREATE_FAILED = -3;
static constexpr JobResult JOB_RESULT_ERROR_TIMEOUT = -4;
static constexpr JobResult JOB_RESULT_ERROR_ERROR_INVALID_STATE = -5;
static constexpr JobResult JOB_RESULT_ERROR_OUT_OF_MEMORY = -6;
static constexpr JobResult JOB_RESULT_ERROR_INVALID_PARAM = -7;
static constexpr JobResult JOB_RESULT_ERROR_JOB_NOTHREAD = -8;
static constexpr JobResult JOB_RESULT_ERROR_UNKNOWN = -999;

struct JobQueueDesc
{
    const char8_t* name = nullptr;
    JobQueuePriority priority = SKR_THREAD_NORMAL;
    uint32_t stack_size = 16 * 1024;
    uint32_t thread_count = 1;
};

struct JobQueueThreadDesc
{
    int32_t __nothing__;
};

struct JobItemDesc
{
    int32_t __nothing__;
};

using EJobStatus = ESkrJobItemStatus;

struct RUNTIME_STATIC_API JobItem
{
    friend struct JobQueue;
    friend struct JobThreadFunction;
    friend struct JobThreadFunctionImpl;
    friend struct JobItemQueue;

public:
    JobItem(const char8_t* name, const JobItemDesc* desc = nullptr) SKR_NOEXCEPT;
    JobItem(const JobItem& src) SKR_NOEXCEPT
        : status( skr_atomic32_load_acquire(&status) )
        , name(src.name), result(src.result), desc(src.desc)
    {

    }
    virtual ~JobItem() SKR_NOEXCEPT;

    // get job item name
    const char8_t* get_name() SKR_NOEXCEPT;

    // obtains JobItem's return code
    // @retval JOB_RESULT_OK if success
    JobResult get_result() SKR_NOEXCEPT;

	// returns if JobItem is executing
    // @retval true if JobItem is not executing
    bool is_none() SKR_NOEXCEPT;

protected:
    // virtual function to describe Job's sub thread processing.
    // @retval JOB_RESULT_OK if success
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

using JobQueueThreadList = eastl::list<JobQueueThread*>;

struct RUNTIME_STATIC_API JobQueue
{
public:
    JobQueue(const JobQueueDesc*) SKR_NOEXCEPT;
	~JobQueue() SKR_NOEXCEPT;

    // initialize JobQueue
    // @retval JOB_RESULT_OK if success
	JobResult initialize(const JobQueueDesc* desc = nullptr) SKR_NOEXCEPT;

    // finalize JobQueue.
    // @retval JOB_RESULT_OK if success
	JobResult finalize() SKR_NOEXCEPT;

    // enqueue JobItem object to job queue.
    // @retval JOB_RESULT_OK if success
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
    // @retval JOB_RESULT_OK if success
	JobResult check() SKR_NOEXCEPT;

    // returns descriptor specified to JobQueue initialize
    void get_descriptor(JobQueueDesc* desc) SKR_NOEXCEPT;

    // cancel all enqueued jobs. 
    // @retval JOB_RESULT_OK if success
    JobResult cancel_all_items() SKR_NOEXCEPT;

private:
    friend struct JobThreadFunction;
    int enqueueCore(JobItem* jobItem, bool isEndJob) SKR_NOEXCEPT;

    skr::text::text queue_name;
    JobQueueThreadList thread_list;
    JobItemQueue* itemList;
    JobQueueDesc desc;

    skr::vector<JobItem*> pending_queue;
    SRWMutex pending_queue_mutex;
    SAtomic32 cancel_requested = 0;
};
}