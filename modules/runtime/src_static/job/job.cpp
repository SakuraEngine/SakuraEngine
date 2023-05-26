#include "async/thread_job.hpp"
#include "job_thread.hpp"
#include "containers/vector.hpp"
#include "misc/defer.hpp"
#include "misc/log.h"

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
            return ASYNC_RESULT_ERROR_INVALID_STATE;
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

        return ASYNC_RESULT_OK;
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
    skr::string name = u8"JobItemQueue";
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
    return ASYNC_RESULT_OK;
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

class JobFinalizeItem final : public JobItem 
{
public:
    JobFinalizeItem() : JobItem(u8"JobFinalizeItem"){}
protected:
    // this job is used to notify the end of the job
    JobResult run() SKR_NOEXCEPT override
    {
        return ASYNC_RESULT_OK;
    }
    void finish(JobResult res) SKR_NOEXCEPT override
    {
        // do nothing
    }  
};

JobQueue::JobQueue(const JobQueueDesc* pDesc) SKR_NOEXCEPT
{
    queue_name = pDesc ? pDesc->name ? pDesc->name : u8"UnknownJobQueue" : u8"UnknownJobQueue";
    skr_init_rw_mutex(&pending_queue_mutex);
    itemList = SkrNew<JobItemQueue>(queue_name.u8_str());
    SKR_ASSERT(itemList);
    initialize(pDesc);
}

JobQueue::~JobQueue() SKR_NOEXCEPT
{
    finalize();
    SkrDelete(itemList);
    skr_destroy_rw_mutex(&pending_queue_mutex);
}

JobResult JobQueue::initialize(const JobQueueDesc* pDesc) SKR_NOEXCEPT
{
    JobResult ret;
    const char8_t* n = pDesc ? pDesc->name : nullptr;
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
            return ASYNC_RESULT_ERROR_OUT_OF_MEMORY;
        }
        skr::string tname = n ? n : u8"UnknownJobQueue";
        auto taftfix = skr::format(u8"_{}"_cuqv, (int32_t)i);
        tname.append(taftfix);
        NamedThreadDesc tdesc = {};
        tdesc.name = tname.u8_str();
        tdesc.priority = desc.priority;
        tdesc.stack_size = desc.stack_size;
        auto t = SkrNew<JobQueueThread>();
        t->initialize(&tdesc);
        SKR_ASSERT(t != nullptr);
        if (t == nullptr)
        {
            SkrDelete(jobfunc);
            return ASYNC_RESULT_ERROR_OUT_OF_MEMORY;
        }

        ret = t->start(jobfunc);
        SKR_ASSERT(ret == ASYNC_RESULT_OK);
        if (ret != ASYNC_RESULT_OK)
        {
            SkrDelete(jobfunc);
            SkrDelete(t);
            return ret;
        }

        thread_list.emplace_back(t);
    }
    return ASYNC_RESULT_OK;
}

int JobQueue::finalize() SKR_NOEXCEPT
{
    // Queue as many JobFinalizeItems as the number of worker threads to finish the worker threads.
    // Since the worker thread that received JobFinalizeItem will always end without taking the next Job,
    // This will terminate all worker threads.
    skr::vector<JobFinalizeItem> finalJobs;
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

    return ASYNC_RESULT_OK;
}

int JobQueue::enqueue(JobItem* jobItem) SKR_NOEXCEPT
{
    // Add to pending queue
    skr_rw_mutex_acuire_w(&pending_queue_mutex);
    SKR_DEFER({skr_rw_mutex_release(&pending_queue_mutex);});
    JobResult ret;
    {
        ret = enqueueCore(jobItem, /*isEndJob=*/false);
    }
    if (ret == ASYNC_RESULT_OK)
    {
        pending_queue.emplace_back(jobItem);
    }
    return ret;
}

bool JobQueue::is_empty() SKR_NOEXCEPT
{
    skr_rw_mutex_acuire_r(&pending_queue_mutex);
    SKR_DEFER({skr_rw_mutex_release(&pending_queue_mutex);});
    return (items_count() == 0 && (pending_queue.size() == 0)) ? true : false;
}

JobResult JobQueue::check() SKR_NOEXCEPT
{
    skr_rw_mutex_acuire_w(&pending_queue_mutex);
    SKR_DEFER({skr_rw_mutex_release(&pending_queue_mutex);});

    auto need_cancel = false;
    need_cancel = skr_atomic32_load_acquire(&cancel_requested);
    if (need_cancel)
    {
        eastl::for_each(pending_queue.begin(), pending_queue.end(), 
        [](auto ptr) {
            if (ptr->is_none() == false) 
            {
                ptr->cancel();
            }
        });
        skr_atomic32_store_release(&cancel_requested, false);
    }

    auto it = pending_queue.begin();
    while (it != pending_queue.end()) {
        // call finish() when finished
        if ((*it)->is_none()) {
            auto jobItemPtr = *it;
            // finish is called while the lock is released
            skr_rw_mutex_release(&pending_queue_mutex);
            jobItemPtr->finish(jobItemPtr->get_result());
            skr_rw_mutex_acuire_w(&pending_queue_mutex);

            // Since enqueue may be done while unlocking, it cannot be used
            // so re-search the list
            it = eastl::find(pending_queue.begin(), pending_queue.end(), jobItemPtr);
            it = pending_queue.erase(it);
        }
        else {
            ++it;
        }
    }

    return ASYNC_RESULT_OK;
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
        return ASYNC_RESULT_ERROR_INVALID_PARAM;
    }

    if (thread_list.size() == 0)
    {
        SKR_ASSERT(0);
        return ASYNC_RESULT_ERROR_JOB_NOTHREAD;
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

JobResult JobQueue::cancel_all_items() SKR_NOEXCEPT
{
    skr_rw_mutex_acuire_w(&pending_queue_mutex);
    SKR_DEFER({skr_rw_mutex_release(&pending_queue_mutex);});
    skr_atomic32_store_release(&cancel_requested, true);
    return ASYNC_RESULT_OK;
}

}