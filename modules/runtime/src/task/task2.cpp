#include "task/task2.hpp"
#include "EASTL/deque.h"
#include "utils/log.h"
#include "utils/make_zeroed.hpp"
#include "utils/function_ref.hpp"

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

    task_t task_t::promise_type::get_return_object()
    {
        return task_t{std::coroutine_handle<promise_type>::from_promise(*this)};
    }

    void task_t::promise_type::unhandled_exception()
    {
        auto exception = std::current_exception();
        if (exception)
        {
            try
            {
                std::rethrow_exception(exception);
            }
            catch (const std::exception& e)
            {
                SKR_LOG_ERROR("Unhandled exception in task: %s", e.what());
            }
        }
    }

    struct Task
    {
        eastl::function<void()> func;
        std::coroutine_handle<> coro;

        Task(eastl::function<void()>&& func)
            : func(std::move(func))
        {
        }

        Task(std::coroutine_handle<>&& coro)
            : coro(std::move(coro))
        {
        }

        Task(nullptr_t)
        {
        }

        void operator()()
        {
            if (func)
                func();
            else
            {
                coro.resume();
            }
        }

        explicit operator bool() const
        {
            return func || coro;
        }
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

        void stop()
        {
            if(!isMainThread)
            {
                enqueue(Task{[this]{shutdown = true;}}, true);
                skr_join_thread(handle);
                skr_destroy_thread(handle);
            }
            else 
            {
                shutdown = true;
            }
        }

        void stealFrom(Worker& other)
        {
        }

        void spinForWork()
        {
            constexpr auto duration = std::chrono::milliseconds(1);
            auto start = std::chrono::high_resolution_clock::now();
            Task stolen(nullptr);
            while (std::chrono::high_resolution_clock::now() - start < duration) 
            {
                for (int i = 0; i < 256; i++)  // Empirically picked magic number!
                {
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
                
                auto numThreads = scheduler->config.numThreads;
                if(numThreads > 1)
                {
                    // Try to steal from other workers
                    for (int i = 0; i < 3; ++i)
                    {
                        int rand = i;
                        while(rand == i)
                            rand = rnd() % numThreads;
                        auto& worker = *(Worker*)scheduler->workers[rand];
                        if(worker.steal(stolen))
                        {
                            SMutexLock lock(work.mutex);
                            work.tasks.push_back(std::move(stolen));
                            work.num++;
                            return;
                        }
                    }
                }

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
                scheduler->spinningWorkers[scheduler->nextEnqueueIndex++ % scheduler->spinningWorkers.size()] = id;
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
            while(!work.tasks.empty() || !work.pinnedTask.empty())
            {
                if(!work.pinnedTask.empty())
                {
                    auto task = std::move(work.pinnedTask.front());
                    work.pinnedTask.pop_front();
                    work.num--;
                    skr_release_mutex(&work.mutex);
                    task();
                    task = Task(nullptr); // destruct before acquiring mutex
                    skr_acquire_mutex(&work.mutex);
                }
                else 
                {
                    auto task = std::move(work.tasks.front());
                    work.tasks.pop_front();
                    work.num--;
                    work.numNoPin--;
                    skr_release_mutex(&work.mutex);
                    task();
                    task = Task(nullptr); // destruct before acquiring mutex
                    skr_acquire_mutex(&work.mutex);
                }
            }
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
            skr_acquire_mutex(&work.mutex);
            enqueueAndUnlock(std::move(task), pinned);
        }

        void enqueueAndUnlock(Task&& task, bool pinned)
        {
            auto notify = work.notifyAdded;
            if(pinned)
            {
                work.pinnedTask.push_back(std::move(task));
            }
            else 
            {
                work.tasks.push_back(std::move(task));
                work.numNoPin++;
            }
            work.num++;
            skr_release_mutex(&work.mutex);
            if (notify)
            {
                skr_wake_condition_var(&work.added);
            }
        }

        bool steal(Task& out) 
        {
            if(work.numNoPin.load() == 0)
                return false;
            if(!skr_try_acquire_mutex(&work.mutex))
                return false;
            if(!work.tasks.empty())
            {
                work.numNoPin--;
                work.num--;
                out = std::move(work.tasks.front());
                work.tasks.pop_front();
                skr_release_mutex(&work.mutex);
                return true;
            }
            skr_release_mutex(&work.mutex);
            return false;
        }

        Worker()
        {
            skr_init_mutex(&work.mutex);
            skr_init_condition_var(&work.added);
        }

        ~Worker()
        {
            skr_destroy_mutex(&work.mutex);
            skr_destroy_condition_var(&work.added);
        }

        struct Work 
        {
            std::atomic<uint64_t> num = 0;
            std::atomic<uint64_t> numNoPin = 0;
            uint64_t numBlockedFibers = 0;
            eastl::deque<Task> pinnedTask;
            eastl::deque<Task> tasks;
            bool notifyAdded = true;
            SConditionVariable added;
            SMutex mutex;

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
        scheduler_t* scheduler = scheduler_t::instance();
        while(true)
        {
            int idx = 0;
            if(workerIdx < 0)
            {
                auto i = --scheduler->nextSpinningWorkerIdx % scheduler->spinningWorkers.size();
                idx = scheduler->spinningWorkers[i].exchange(-1);
                if(idx < 0)
                    idx = scheduler->nextEnqueueIndex++ % (scheduler->workers.size() - 1) + 1;
            }
            else 
            {
                SKR_ASSERT((uint32_t)workerIdx < scheduler->workers.size());
                idx = workerIdx;
            }
            auto& worker = *(Worker*)scheduler->workers[idx];
            if(skr_try_acquire_mutex(&worker.work.mutex))
            {
                worker.enqueueAndUnlock(std::move(task), workerIdx >= 0);
                return;
            }
        }
    }


    scheduler_t::Awaitable::Awaitable(scheduler_t& scheduler, event_t event, int workerIdx)
        : scheduler(scheduler), event(std::move(event)), workerIdx(workerIdx)
    {
    }

    void scheduler_t::schedule(eastl::function<void ()> function)
    {
        enqueue(Task(std::move(function)), -1);
    }

    void scheduler_t::schedule(task_t&& task)
    {
        std::coroutine_handle<> coroutine = task.coroutine;
        task.coroutine = nullptr;
        enqueue(Task(std::move(coroutine)), -1);
    }

    bool scheduler_t::Awaitable::await_ready() const
    {
        return event.done();
    }

    void scheduler_t::Awaitable::await_suspend(std::coroutine_handle<> handle)
    {   
        SMutexLock guard(event.state->mutex);
        event.state->numWaiting++;
        if(event.state->signalled)
        {
            handle.resume();
            event.state->numWaiting--;
            return;
        }
        event.state->waiters.push_back(handle);
        event.state->workerIndices.push_back(workerIdx);
    }

    scheduler_t::Awaitable scheduler_t::wait(event_t event, bool pinned)
    {
        auto worker = currentWorker;
        //SKR_ASSERT(worker != nullptr);
        return {*this, event, pinned && worker ? (int)worker->id : -1};
    }

    void scheduler_t::sync(event_t event)
    {
        auto worker = currentWorker;
        SKR_ASSERT(worker == nullptr);
        worker = (Worker*)mainWorker;
        SMutexLock guard(worker->work.mutex);
        while(!event.done())
        {
            skr_release_mutex(&worker->work.mutex);
            worker->spinForWork();
            skr_acquire_mutex(&worker->work.mutex);
            worker->runUntilIdle();
        }
    }

    void event_t::notify()
    {
        if(!state)
            return;
        SMutexLock guard(state->mutex);
        state->signalled = true;
        skr_wake_all_condition_vars(&state->cv);
        for(uint32_t i=0; i<state->waiters.size(); ++i)
        {
            enqueue(Task{state->waiters[i]}, state->workerIndices[i]);
        }
        state->waiters.clear();
        state->workerIndices.clear();
        state->numWaiting = 0;
        state->numWaitingOnCondition = 0;
    }

    void event_t::reset()
    {
        if(!state)
            return;
        SMutexLock guard(state->mutex);
        state->signalled = false;
    }

    void scheduler_t::initialize(const scheudler_config_t & cfg)
    {
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
        SKR_ASSERT(initialized);
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