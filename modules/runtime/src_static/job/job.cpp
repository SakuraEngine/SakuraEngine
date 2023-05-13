#include "job/thread_job.hpp"
#include "job_thread.hpp"
#include "containers/vector.hpp"

namespace skr
{
struct JobThreadFunctionImpl : public JobThreadFunction
{
public:
    JobThreadFunctionImpl(JobItemQueue* itemQueue)
        : m_item(nullptr)
        , m_queue(itemQueue)
    {
        
    }
    virtual ~JobThreadFunctionImpl()
    {

    }
protected:
    JobResult run() SKR_NOEXCEPT override;
private:
    JobItem*		m_item;
    JobItemQueue*	m_queue;
};

struct JobItemQueue
{
    friend struct JobQueue;

public:
    JobItemQueue(const char8_t* name) 
        : waiting_workers_count(0) , name(name) , is_end_job_queued(false)
    {
        cond = SkrNew<JobQueueCond>(u8"SampleUtilJobItemQueueCond");					
        SKR_ASSERT(cond != nullptr);

    }

    ~JobItemQueue()
    {
        SkrDelete(cond);
    }

    JobResult push(JobItem* jobItem, bool isEndJob = false)
    {
        cond->lock();
        const auto endJobEnqueud = skr_atomic32_load_acquire(&is_end_job_queued);
        if (endJobEnqueud && !isEndJob)
        {
            cond->unlock();
            return JOB_RESULT_ERROR_ERROR_INVALID_STATE;
        }

        // update the status of JobItem together with the state of the queue
        skr_atomic32_store_release(&jobItem->status, isEndJob ? kJobItemStatusFinishJob : kJobItemStatusWaiting);
        list_runnable.emplace_back(jobItem);

        // end job is queued, only end job is accepted
        skr_atomic32_store_release(&is_end_job_queued, isEndJob);

        const auto c = skr_atomic32_load_acquire(&waiting_workers_count);
        if (c) 
        {
            cond->signal();
        }

        cond->unlock();

        return JOB_RESULT_OK;
    }

    void erase(JobItem* jobItem)
    {
        cond->lock();
        SKR_ASSERT(jobItem);
        SKR_ASSERT(jobItem->status != kJobItemStatusNone);

        bool isFound = false;
        for (auto i = list_consumed.begin(); i != list_consumed.end(); ++i)
        {
            if ((*i) == jobItem) 
            {
                // update the status of JobItem together with the state of the queue
                list_consumed.erase(i);
                skr_atomic32_store_release(&jobItem->status, kJobItemStatusNone);
                isFound = true;
                break;
            }
        }
        SKR_ASSERT(isFound);

        cond->unlock();
    }

    JobItem* getRunnableJobItem()
    {
        cond->lock();

        // wait until the notification is queued
        while (list_runnable.size() == 0) 
        {
            skr_atomic32_add_relaxed(&waiting_workers_count, 1);

            cond->wait();

            skr_atomic32_add_relaxed(&waiting_workers_count, -1);
        }

        JobItem* jobItem = nullptr;
        // previous jobItem is moved to consumed list
        jobItem = *list_runnable.begin();
        list_runnable.erase(list_runnable.begin());
        list_consumed.emplace_back(jobItem);

        SKR_ASSERT(jobItem != nullptr);
        SKR_ASSERT(jobItem->status == kJobItemStatusWaiting || jobItem->status == kJobItemStatusFinishJob);

        // update the status of JobItem together with the state of the queue
        ESkrJobItemStatus statusWaiting = kJobItemStatusWaiting; (void)statusWaiting;
        skr_atomic32_cas_relaxed(&jobItem->status, statusWaiting, kJobItemStatusRunning);
        // jobItem->status.compare_exchange_strong((int&)statusWaiting, kJobItemStatusRunning, std::memory_order_acq_rel, std::memory_order_acquire);

        cond->unlock();

        return jobItem;
    }

    uint64_t numItems()
    {
        cond->lock();
        const auto num = list_runnable.size() + list_consumed.size();
        cond->unlock();
        return num;
    }

    SAtomic32 waiting_workers_count = 0;
    skr::text::text name = u8"JobItemQueue";
    SAtomic32 is_end_job_queued = false;

    skr::vector<JobItem*> list_runnable;
    skr::vector<JobItem*> list_consumed;
    JobQueueCond* cond = nullptr;

};

JobResult JobThreadFunctionImpl::run() SKR_NOEXCEPT
{
    bool isEndJobArrived = false;
    while (false == isEndJobArrived)
    {
        m_item = m_queue->getRunnableJobItem();

        if (m_item)
        {
            const auto itemStatus = skr_atomic32_load_acquire(&m_item->status);
            SKR_ASSERT((itemStatus == kJobItemStatusRunning) || (itemStatus == kJobItemStatusFinishJob));

            // exit the loop when the end job is arrived
            isEndJobArrived = (itemStatus == kJobItemStatusFinishJob);

            m_item->result = m_item->run();

            // delete the JobItem which has been executed
            m_queue->erase(m_item);
            m_item = nullptr;
        }
    }
    return JOB_RESULT_OK;
}

JobItem::JobItem(const char8_t* name, const JobItemDesc* pDesc) SKR_NOEXCEPT
    : status(kJobItemStatusNone)
    , name(name)
    , result(0)
{
    if (pDesc) {
        desc = *pDesc;
    }
}

const char8_t* JobItem::get_name() SKR_NOEXCEPT
{
    return name.u8_str();
}

JobItem::~JobItem() SKR_NOEXCEPT
{

}

bool JobItem::is_none() SKR_NOEXCEPT
{
    return (skr_atomic32_load_acquire(&status) == kJobItemStatusNone);
}

int JobItem::get_result() SKR_NOEXCEPT
{
    return result;
}

class JobFinalizeItem : public JobItem 
{
public:
    JobFinalizeItem() : JobItem(u8"SampleUtilDummyJob"){}
protected:
    // this job is used to notify the end of the job
    JobResult run() SKR_NOEXCEPT override
    {
        return JOB_RESULT_OK;
    }
};


JobQueue::JobQueue() SKR_NOEXCEPT
    : name(u8"")
{
    itemList = SkrNew<JobItemQueue>(u8"SampleUtilJobItemQueue");
    SKR_ASSERT(itemList);
}

JobQueue::JobQueue(const JobQueueDesc* pDesc) SKR_NOEXCEPT
    : name(u8"")
{
    itemList = SkrNew<JobItemQueue>(u8"SampleUtilJobItemQueue");
    SKR_ASSERT(itemList);

    initialize(u8"SampleUtilJobQueue", pDesc);
}

JobQueue::~JobQueue() SKR_NOEXCEPT
{
    finalize();
    SkrDelete(itemList);
}

JobResult JobQueue::initialize(const char8_t *name, const JobQueueDesc* pDesc) SKR_NOEXCEPT
{
    JobResult ret;
    if (pDesc != nullptr)
    {
        desc = *pDesc;
    }
    for (uint32_t i = 0; i < desc.thread_count; ++i)
    {
        JobThreadFunction* jobfunc = SkrNew<JobThreadFunctionImpl>(itemList);
        SKR_ASSERT(jobfunc != nullptr);
        if (jobfunc == nullptr)
        {
            return JOB_RESULT_ERROR_OUT_OF_MEMORY;
        }

        auto *t = SkrNew<JobQueueThread>(name, desc.priority, desc.stack_size);
        SKR_ASSERT(t != nullptr);
        if (t == nullptr)
        {
            SkrDelete(jobfunc);
            return JOB_RESULT_ERROR_OUT_OF_MEMORY;
        }

        ret = t->start(jobfunc);
        SKR_ASSERT(ret == JOB_RESULT_OK);
        if (ret != JOB_RESULT_OK)
        {
            SkrDelete(jobfunc);
            SkrDelete(t);
            return ret;
        }

        thread_list.emplace_back(t);
    }
    return JOB_RESULT_OK;
}

int JobQueue::finalize() SKR_NOEXCEPT
{
    // Queue as many JobFinalizeItems as the number of worker threads to finish the worker threads.
    // Since the worker thread that received JobFinalizeItem will always end without taking the next Job,
    // This will terminate all worker threads.
    std::vector<JobFinalizeItem> finalJobs;
    finalJobs.reserve(thread_list.size());
    for (int i = 0; i < thread_list.size(); ++i)
    {
        finalJobs.emplace_back();
        enqueueCore(&finalJobs.back(), /*isEndJob=*/true); // Normal enqueue will fail after this point
    }

    // wait for worker thread to finish。
    for (auto *pWorkerThread : thread_list)
    {
        pWorkerThread->join();

        SkrDelete(pWorkerThread->get_function());
        SkrDelete(pWorkerThread);
    }
    thread_list.clear();

    return JOB_RESULT_OK;
}

int JobQueue::enqueue(JobItem	*jobItem) SKR_NOEXCEPT
{
    // You can't add end Jobs from the outside.
    return enqueueCore(jobItem, /*isEndJob=*/false);
}

void JobQueue::check() SKR_NOEXCEPT
{
}

void JobQueue::get_descriptor(JobQueueDesc* pDesc) SKR_NOEXCEPT
{
    SKR_ASSERT(pDesc != nullptr);
    if (pDesc != nullptr)
    {
        *pDesc = desc;
    }
}

int JobQueue::enqueueCore(JobItem* jobItem, bool isEndJob) SKR_NOEXCEPT
{
    SKR_ASSERT(jobItem->status == kJobItemStatusNone);

    if (skr_atomic32_load_acquire(&jobItem->status) != kJobItemStatusNone)
    {
        return JOB_RESULT_ERROR_INVALID_PARAM;
    }

    if (thread_list.size() == 0)
    {
        SKR_ASSERT(0);
        return JOB_RESULT_ERROR_JOB_NOTHREAD;
    }

    return itemList->push(jobItem, isEndJob);
}

uint32_t JobQueue::items_count() const SKR_NOEXCEPT
{
    return (uint32_t)itemList->numItems();
}

void JobQueue::wait_empty() const SKR_NOEXCEPT
{
    while (items_count() > 0)
    {
        skr_thread_sleep(10);
    }
}

void JobQueue::change_priority(JobQueuePriority priority) SKR_NOEXCEPT
{
    for (auto *pWorkerThread : thread_list)
    {
        pWorkerThread->change_priority(priority);
    }
}

}