#if __cpp_impl_coroutine

#include "task/task2.hpp"
#include "EASTL/deque.h"
#include "utils/log.h"
#include "utils/make_zeroed.hpp"
#include "utils/function_ref.hpp"
#include "utils/defer.hpp"
#include "containers/atomic_queue/atomic_queue.h"
#include "containers/concurrent_queue.h"

inline void nop() {
#if defined(_WIN32)
  __nop();
#else
  __asm__ __volatile__("nop");
#endif
}

// https://en.wikipedia.org/wiki/Xorshift
class FastRnd {
    public:
    inline uint64_t operator()() {
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    return x;
    }

    private:
    uint64_t x = std::chrono::system_clock::now().time_since_epoch().count();
};

namespace skr
{
namespace task2
{
    thread_local scheduler_t* currentScheduler = nullptr;

    scheduler_t* scheduler_t::instance()
    {
        return currentScheduler;
    }

    void set_instance(scheduler_t* scheduler)
    {
        currentScheduler = scheduler;
    }

    scheudler_config_t::scheudler_config_t()
    {
        numThreads = skr_cpu_cores_count();
    }

    skr_task_t skr_task_t::promise_type::get_return_object()
    {
        return skr_task_t{std::coroutine_handle<promise_type>::from_promise(*this)};
    }

    void skr_task_t::promise_type::unhandled_exception()
    {
        SKR_ASSERT(false);
        auto exception = std::current_exception();
    }

    struct Task
    {
        eastl::function<void()> func;
        std::coroutine_handle<skr_task_t::promise_type> coro;

        Task() {}
        Task(nullptr_t) {}

        Task(eastl::function<void()>&& func)
            : func(std::move(func))
        {
        }

        Task(std::coroutine_handle<skr_task_t::promise_type>&& coro)
            : coro(std::move(coro))
        {
            SKR_ASSERT(!this->coro.done());
            coro = nullptr;
        }

        Task(Task&& other)
            : func(std::move(other.func))
            , coro(std::move(other.coro))
        {
            SKR_ASSERT(func || !this->coro.done());
            other.coro = nullptr;
        }

        Task& operator=(Task&& other)
        {
            func = std::move(other.func);
            coro = std::move(other.coro);
            SKR_ASSERT(func || !this->coro.done());
            other.coro = nullptr;
            return *this;
        }

        void operator()()
        {
            SKR_ASSERT(*this);
            if (func)
            {
                func();
                func = {};
            }
            else
            {
#ifdef TRACY_ENABLE
                // the first fiber enter is in coroutine body
                if(coro.promise().name != nullptr)
                    TracyFiberEnter(coro.promise().name);
#endif
                SKR_ASSERT(!coro.done());
                coro.resume();
            }
        }

        explicit operator bool() const
        {
            return func || coro;
        }
    };

    struct Task2ConcurrentQueueTraits : public skr::ConcurrentQueueDefaultTraits
    {
        static constexpr const char* kTask2QueueName = "";
        static const bool RECYCLE_ALLOCATED_BLOCKS = true;
        static inline void* malloc(size_t size) { return sakura_mallocN(size, kTask2QueueName); }
        static inline void free(void* ptr) { return sakura_freeN(ptr, kTask2QueueName); }
    };

    struct WorkQueue
    {
#ifdef USE_ATOMIC_QUEUE
        atomic_queue::AtomicQueue2<Task, 1024> queue;
        bool pop(Task& task) { return queue.try_pop(task); }
        bool steal(Task& task) { return queue.try_pop(task); }
        void push(Task&& task) { queue.push(std::move(task)); }
#else
        skr::ConcurrentQueue<Task, Task2ConcurrentQueueTraits> queue;
        bool pop(Task& task) { return queue.try_dequeue(task); }
        bool steal(Task& task) { return queue.try_dequeue(task); }
        void push(Task&& task) { queue.enqueue(std::move(task)); }
#endif
    };

    void enqueue(Task&& task, int workerIdx);
    thread_local struct Worker* currentWorker = nullptr;
    struct Worker
    {

        void start()
        {
            if(!isMainThread)
            {
                desc = make_zeroed<SThreadDesc>();
                desc.pData = this;
                desc.pFunc = +[](void* pData)
                {
                    Worker* pWorker = (Worker*)pData;
                    set_instance(pWorker->scheduler);
                    currentWorker = pWorker;
                    {
                        SMutexLock lock(pWorker->work.mutex);
                        pWorker->run();
                    }
                    currentWorker = nullptr;
                    set_instance(nullptr);
                };
                skr_init_thread(&desc, &handle);
            }
            else 
            {
                //do nothing
            }
        }

        void signalStop()
        {
            if(!isMainThread)
            {
                enqueue(Task{[this]{shutdown = true;}}, true);
            }
            else 
            {
                shutdown = true;
            }
        }

        void stop()
        {
            if(!isMainThread)
            {
                skr_join_thread(handle);
                skr_destroy_thread(handle);
            }
            else 
            {
                //do nothing
            }
        }

        int stealWork(int rand)
        {
            Task stolen(nullptr);
            auto numThreads = scheduler->config.numThreads;
            if(numThreads > 1)
            {
                ZoneScopedN("Worker::StealWork");
                // Try to steal from other workers
                while(rand == id)
                    rand = rnd() % numThreads;
                auto& worker = *(Worker*)scheduler->workers[rand];
                if(worker.steal(stolen))
                {
                    stolen();
                    return rand;
                }
                else 
                {
                    return id;
                }
            }
            return id;
        }

        void spinForWork()
        {
            ZoneScopedN("Worker::SpinForWork");
            constexpr auto duration = std::chrono::milliseconds(1);
            auto start = std::chrono::high_resolution_clock::now();
            int lastStolen = id;
            while (std::chrono::high_resolution_clock::now() - start < duration) 
            {
                if (work.num > 0) {
                    return;
                }
                if(lastStolen == id)
                {
                    //spin
                    for (int i = 0; i < 256; i++)  // Empirically picked magic number!
                    {
                        // atomic_queue::spin_loop_pause();
                        // clang-format off
                        nop(); nop(); nop(); nop(); nop(); nop(); nop(); nop();
                        nop(); nop(); nop(); nop(); nop(); nop(); nop(); nop();
                        nop(); nop(); nop(); nop(); nop(); nop(); nop(); nop();
                        nop(); nop(); nop(); nop(); nop(); nop(); nop(); nop();
                        // clang-format on
                        if (work.num > 0) {
                            return;
                        }
                    }
                }
                lastStolen = stealWork(lastStolen);
                std::this_thread::yield();
            }
        }

        void waitForWork()
        {
            if(work.num > 0)
                return;
            if(!isMainThread)
            {
                //report to scheduler that we are spinning
                scheduler->spinningWorkers[scheduler->nextSpinningWorkerIdx++ % scheduler->spinningWorkers.size()] = id;
                skr_release_mutex(&work.mutex);
                spinForWork();
                skr_acquire_mutex(&work.mutex);
            }
            work.wait([this]() {
                return work.num > 0 || shutdown;
            });
        }

        void runUntilIdle() 
        {
            ZoneScopedN("Worker::RunUntilIdle");
            while(!work.pinnedTask.empty())
            {
                auto task = std::move(work.pinnedTask.front());
                work.pinnedTask.pop_front();
                work.num--;
                skr_release_mutex(&work.mutex);
                task();
                skr_acquire_mutex(&work.mutex);
            }
            skr_release_mutex(&work.mutex);
            Task task(nullptr);
            while(work.tasks.pop(task))
            {
                work.num--;
                task();
            }
            skr_acquire_mutex(&work.mutex);
        }

        void run()
        {
            if (!isMainThread)
            {
                //initial with wait to avoid spinning
                work.wait([this]() {
                    return work.num > 0 || shutdown;
                });
            }
            while(!shutdown || work.num > 0)
            {
                waitForWork();
                runUntilIdle();
            }
        }

        void enqueue(Task&& task, bool pinned)
        {
            if(!pinned)
            {
                work.num++;
                auto notify = work.notifyAdded;
                work.tasks.push(std::move(task));
                if (notify)
                {
                    skr_wake_condition_var(&work.added);
                }
                return;
            }
            else 
            {
                skr_acquire_mutex(&work.mutex);
                enqueuePinnedAndUnlock(std::move(task));
            }
        }

        void enqueuePinnedAndUnlock(Task&& task)
        {
            ZoneScopedN("EnqueueTaskWorker");
            auto notify = work.notifyAdded;
            work.pinnedTask.push_back(std::move(task));
            work.num++;
            skr_release_mutex(&work.mutex);
            if (notify)
            {
                skr_wake_condition_var(&work.added);
            }
        }

        bool steal(Task& out) 
        {
            bool result = work.tasks.steal(out);
            if (result) 
            {
                work.num--;
            }
            return result;
        }

        Worker()
            
        {
        }

        ~Worker()
        {
        }

        struct Work 
        {
            std::atomic<uint64_t> num = 0;
            eastl::deque<Task, eastl::allocator_sakura, 128> pinnedTask;
            WorkQueue tasks;
            bool notifyAdded = true;
            SConditionVariable added;
            SMutex mutex;
            Work()
            {
                skr_init_mutex(&mutex);
                skr_init_condition_var(&added);
            }
            ~Work()
            {
                skr_destroy_mutex(&mutex);
                skr_destroy_condition_var(&added);
            }

            template<class F>
            void wait(F&& f)
            {
                notifyAdded = true;
                while(!f())
                {
                    skr_wait_condition_vars(&added, &mutex, TIMEOUT_INFINITE);
                }
                notifyAdded = false;
            }
        } work;
        SThreadDesc desc;
        SThreadHandle handle;
        bool isMainThread = false;
        scheduler_t* scheduler = nullptr;
        bool shutdown = false;
        uint32_t id;
        FastRnd rnd;
    };
    

    void enqueue(Task&& task, int workerIdx)
    {
        //ZoneScopedN("EnqueueTask");
        scheduler_t* scheduler = scheduler_t::instance();
        SKR_ASSERT(scheduler != nullptr);
        size_t workerCount = scheduler->config.numThreads;
        while(true)
        {
            int idx = 0;
            if(workerIdx < 0)
            {
                auto i = --scheduler->nextSpinningWorkerIdx % scheduler->spinningWorkers.size();
                idx = scheduler->spinningWorkers[i].exchange(-1);
                if(idx < 0)
                    idx = scheduler->nextEnqueueIndex++ % (workerCount - 1) + 1;
            }
            else 
            {
                SKR_ASSERT((uint32_t)workerIdx < workerCount);
                idx = workerIdx;
            }
            auto& worker = *(Worker*)scheduler->workers[idx];
            if(workerIdx < 0)
            {
                worker.enqueue(std::move(task), false);
                return;
            }
            else if(skr_try_acquire_mutex(&worker.work.mutex))
            {
                worker.enqueuePinnedAndUnlock(std::move(task));
                return;
            }
        }
    }

    void scheduler_t::schedule(eastl::function<void ()>&& function)
    {
        enqueue(Task(std::move(function)), -1);
    }

    void scheduler_t::schedule(skr_task_t&& task)
    {
        std::coroutine_handle<skr_task_t::promise_type> coroutine = task.coroutine;
        task.coroutine = nullptr;
        enqueue(Task(std::move(coroutine)), -1);
    }

    scheduler_t::EventAwaitable::EventAwaitable(scheduler_t& scheduler, event_t event, int workerIdx)
        : scheduler(scheduler), event(std::move(event)), workerIdx(workerIdx)
    {
    }

    bool scheduler_t::EventAwaitable::await_ready() const
    {
        return event.done();
    }

    bool scheduler_t::EventAwaitable::await_suspend(std::coroutine_handle<skr_task_t::promise_type> handle)
    {   
        SMutexLock guard(event.state->mutex);
        if(event.state->signalled)
            return false;
#ifdef TRACY_ENABLE
        if(handle.promise().name != nullptr)
            TracyFiberLeave;
#endif
        event.state->cv.add_waiter(handle, workerIdx);
        return true;
    }

    scheduler_t::EventAwaitable co_wait(event_t event, bool pinned)
    {
        auto scheduler = scheduler_t::instance();
        SKR_ASSERT(scheduler != nullptr);
        auto worker = currentWorker;
        //SKR_ASSERT(worker != nullptr);
        return {*scheduler, event, pinned && worker ? (int)worker->id : -1};
    }

    void wait(event_t event)
    {
        auto scheduler = scheduler_t::instance();
        SKR_ASSERT(scheduler == nullptr); //must use outside of scheduler
        while(!event.done())
        {
            event.state->cv.wait(event.state->mutex);
        }
    }

    scheduler_t::CounterAwaitable::CounterAwaitable(scheduler_t& scheduler, counter_t counter, int workerIdx)
        : scheduler(scheduler), counter(std::move(counter)), workerIdx(workerIdx)
    {
    }

    bool scheduler_t::CounterAwaitable::await_ready() const
    {
        return counter.done();
    }

    bool scheduler_t::CounterAwaitable::await_suspend(std::coroutine_handle<skr_task_t::promise_type> handle)
    {   
        SMutexLock guard(counter.state->mutex);
        if(counter.state->count == 0)
            return false;
#ifdef TRACY_ENABLE
        if(handle.promise().name != nullptr)
            TracyFiberLeave;
#endif
        counter.state->cv.add_waiter(handle, workerIdx);
        return true;
    }

    scheduler_t::CounterAwaitable co_wait(counter_t counter, bool pinned)
    {
        auto scheduler = scheduler_t::instance();
        SKR_ASSERT(scheduler != nullptr);
        auto worker = currentWorker;
        //SKR_ASSERT(worker != nullptr);
        return {*scheduler, counter, pinned && worker ? (int)worker->id : -1};
    }

    void wait(counter_t counter)
    {
        auto scheduler = scheduler_t::instance();
        SKR_ASSERT(scheduler == nullptr); //must use outside of scheduler
        while(!counter.done())
        {
            counter.state->cv.wait(counter.state->mutex);
        }
    }

    void scheduler_t::sync(event_t event)
    {
        ZoneScopedN("SyncEvent");
        auto worker = currentWorker;
        SKR_ASSERT(worker == nullptr);
        worker = (Worker*)mainWorker;
        SMutexLock guard(worker->work.mutex);
        int i = 0;
        while(!event.done())
        {
            skr_release_mutex(&worker->work.mutex);
            i = worker->stealWork(i);
            skr_acquire_mutex(&worker->work.mutex);
            worker->runUntilIdle();
        }
    }

    void scheduler_t::sync(counter_t counter)
    {
        ZoneScopedN("SyncEvent");
        auto worker = currentWorker;
        SKR_ASSERT(worker == nullptr);
        worker = (Worker*)mainWorker;
        SMutexLock guard(worker->work.mutex);
        int i = 0;
        while(!counter.done())
        {
            skr_release_mutex(&worker->work.mutex);
            i = worker->stealWork(i);
            skr_acquire_mutex(&worker->work.mutex);
            worker->runUntilIdle();
        }
    }

    void condvar_t::add_waiter(std::coroutine_handle<skr_task_t::promise_type> handle, int workerIdx)
    {
        ++numWaiting;
        waiters.push_back(handle);
        workerIndices.push_back(workerIdx);
    }

    void condvar_t::wait(SMutex& mutex)
    {
        ++numWaitingOnCondition;
        skr_wait_condition_vars(&cv, &mutex, TIMEOUT_INFINITE);
        --numWaitingOnCondition;
    }

    void condvar_t::notify()
    {
        skr_wake_all_condition_vars(&cv);
        for(uint32_t i=0; i<waiters.size(); ++i)
        {
            enqueue(Task{std::move(waiters[i])}, workerIndices[i]);
        }
        waiters.clear();
        workerIndices.clear();
        numWaiting = 0;
    }

    void event_t::notify()
    {
        if(!state)
            return;
        SMutexLock guard(state->mutex);
        state->signalled = true;
        state->cv.notify();
    }

    void event_t::reset()
    {
        if(!state)
            return;
        SMutexLock guard(state->mutex);
        state->signalled = false;
    }

    void counter_t::add(uint32_t count)
    {
        if(!state)
            return;
        state->count += count;
        if(state->inverse)
        {
            SMutexLock guard(state->mutex);
            state->cv.notify();
        }
    }

    bool counter_t::decrease()
    { 
        SKR_ASSERT(state->count > 0);
        auto count = --state->count;
        if(state->inverse)
            return false;
        if(count == 0)
        {
            SMutexLock guard(state->mutex);
            state->cv.notify();
            return true;
        }
        return false;
    }

    void scheduler_t::initialize(const scheudler_config_t & cfg)
    {
        ZoneScopedN("Scheduler::Initialize");
        config = cfg;
        for(size_t i = 0; i < spinningWorkers.size(); ++i)
            spinningWorkers[i] = -1;
        for(size_t i = 0; i < cfg.numThreads; ++i)
        {
            auto worker = SkrNew<Worker>();
            worker->id = i;
            worker->scheduler = this;
            worker->isMainThread = i == 0;
            worker->shutdown = false;
            workers[i] = worker;
        }
        for(size_t i = 0; i < cfg.numThreads; ++i)
            ((Worker*)workers[i])->start();
        mainWorker = (Worker*)workers[0];
        initialized = true;
    }

    void scheduler_t::shutdown()
    {
        ZoneScopedN("Scheduler::Shutdown");
        SKR_ASSERT(initialized);
        for(size_t i = 0; i < config.numThreads; ++i)
            ((Worker*)workers[i])->signalStop();
        for(size_t i = 0; i < config.numThreads; ++i)
            ((Worker*)workers[i])->stop();
        for(size_t i = 0; i < config.numThreads; ++i)
            SkrDelete((Worker*)workers[i]);

    }

    void scheduler_t::bind()
    {
        SKR_ASSERT(instance() == nullptr);
        SKR_ASSERT(!binded);
        binded = true;
        set_instance(this);
    }

    void scheduler_t::unbind()
    {
        SKR_ASSERT(instance() == this);
        SKR_ASSERT(binded);
        binded = false;
        set_instance(nullptr);
    }
}
}

#endif