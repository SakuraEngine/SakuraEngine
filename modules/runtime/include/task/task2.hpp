#pragma once
#include "platform/configure.h"
#include "utils/defer.hpp"
#include "platform/debug.h"
#include "EASTL/functional.h"
#include "platform/thread.h"
#include "containers/sptr.hpp"
#include "containers/vector.hpp"
#include <array>
#ifdef __has_include
#if __has_include(<coroutine>)
    #include <coroutine>
#else
    #include <experimental/coroutine>
    //UB
    namespace std
    {
        template<class T>
        using coroutine_handle = experimental::coroutine_handle<T>;
        using suspend_always = experimental::suspend_always;
        using suspend_never = experimental::suspend_never;
    }
#endif
#endif

namespace skr
{
namespace task2
{
    struct RUNTIME_API scheudler_config_t
    {
        scheudler_config_t();
        bool setAffinity = true;
        uint32_t numThreads = 0;
    };
#ifdef TRACY_ENABLE
    struct task_name_t
    {
        const char* name;
    };
    #define TracyTask(name) TracyFiberEnter(name); co_yield task_name_t{name}
#else
    #define TracyTask(name)
#endif
    struct skr_task_t
    {
        struct promise_type
        {
            RUNTIME_API skr_task_t get_return_object();
            std::suspend_always initial_suspend() { return {}; }
            std::suspend_always final_suspend() noexcept { return {}; }
            void return_void() {}
            RUNTIME_API void unhandled_exception();
#ifdef TRACY_ENABLE
            std::suspend_never yield_value(task_name_t tn) { name = tn.name; return {}; }
#endif

            void* operator new(size_t size) { return sakura_malloc(size); }
            void operator delete(void* ptr, size_t size) { sakura_free(ptr); }
            std::exception_ptr exception = nullptr;
#ifdef TRACY_ENABLE
            const char* name = nullptr;
#endif
        };
        skr_task_t(std::coroutine_handle<promise_type> coroutine) : coroutine(coroutine) {}
        skr_task_t(skr_task_t&& other) : coroutine(other.coroutine) { other.coroutine = nullptr; }
        skr_task_t() = default;
        skr_task_t& operator=(skr_task_t&& other) { coroutine = other.coroutine; other.coroutine = nullptr; return *this; }
        ~skr_task_t() { if(coroutine) coroutine.destroy(); }
        void resume() { coroutine.resume(); }
        bool done() const { return coroutine.done(); }
        explicit operator bool() const { return (bool)coroutine; }
        std::coroutine_handle<promise_type> coroutine;
    };

    struct condvar_t
    {
        SConditionVariable cv;
        skr::vector<std::coroutine_handle<skr_task_t::promise_type>> waiters;
        skr::vector<int> workerIndices;
        std::atomic<int> numWaiting = {0};
        std::atomic<int> numWaitingOnCondition = {0};
        condvar_t()
        {
            skr_init_condition_var(&cv);
        }
        ~condvar_t()
        {
            skr_destroy_condition_var(&cv);
        }
        void notify();
        void add_waiter(std::coroutine_handle<skr_task_t::promise_type> waiter, int workerIndex);
        void wait(SMutex& mutex);
    };
    struct event_t
    {
        struct State
        {
            condvar_t cv;
            SMutex mutex;
            bool signalled = false;
            State()
            {
                skr_init_mutex(&mutex);
            }
            ~State()
            {
                skr_destroy_mutex(&mutex);
            }
        };

        event_t()
            : state(SkrNew<State>())
        {
        }
        event_t(const event_t& other) : state(other.state) {}
        event_t(event_t&& other) : state(std::move(other.state)) {}
        event_t& operator=(const event_t& other) { state = other.state; return *this;}
        event_t& operator=(event_t&& other) { state = std::move(other.state); return *this;}
        event_t(std::nullptr_t)
            : state(nullptr)
        {
        }
        ~event_t()
        {
        }
        RUNTIME_API void notify();
        RUNTIME_API void reset();
        size_t hash() const { return (size_t)state.get(); }
        bool done() const 
        { 
            if(!state) 
                return true;
            SMutexLock guard(state->mutex);
            return state->signalled; 
        }
        explicit operator bool() const { return (bool)state; }
        bool operator==(const event_t& other) const { return state == other.state; }

        event_t(SPtr<State> state) : state(std::move(state)) {}
        SPtr<State> state;
    };
    struct weak_event_t
    {
        weak_event_t(event_t& e) : state(e.state) {}
        weak_event_t(const weak_event_t& other) = default;
        weak_event_t(std::nullptr_t) {}
        ~weak_event_t() = default;
        event_t lock() const { return event_t{ state.lock() }; }
        bool expired() const { return state.expired(); }

        SWeakPtr<event_t::State> state;
    };
    struct counter_t
    {
        struct State
        {
            SMutex mutex;
            std::atomic<uint32_t> count = 0;
            bool inverse = false;
            condvar_t cv;
            State()
            {
                skr_init_mutex(&mutex);
            }
            ~State()
            {
                skr_destroy_mutex(&mutex);
            }
        };

        counter_t()
            : state(SkrNew<State>())
        {
        }
        counter_t(bool inverse)
            : state(SkrNew<State>())
        {
            state->inverse = inverse;
        }
        counter_t(std::nullptr_t)
            : state(nullptr)
        {
        }
        counter_t(const counter_t& other) : state(other.state) {}
        counter_t(counter_t&& other) : state(std::move(other.state)) {}
        counter_t& operator=(const counter_t& other) { state = other.state; return *this;}
        counter_t& operator=(counter_t&& other) { state = std::move(other.state); return *this;}
        ~counter_t()
        {
        }
        RUNTIME_API void add(uint32_t count);
        size_t hash() const { return (size_t)state.get(); }
        RUNTIME_API bool decrease();
        bool done() const 
        { 
            if(!state) 
                return true;
            return (state->count == 0) == (!state->inverse); 
        }
        explicit operator bool() const { return (bool)state; }
        bool operator==(const counter_t& other) const { return state == other.state; }

        counter_t(SPtr<State> state) : state(std::move(state)) {}
        SPtr<State> state;
    };

    struct weak_counter_t
    {
        weak_counter_t(counter_t& e) : state(e.state) {}
        weak_counter_t(const weak_counter_t& other) = default;
        weak_counter_t(std::nullptr_t) {}
        ~weak_counter_t() = default;
        counter_t lock() const { return counter_t{ state.lock() }; }
        bool expired() const { return state.expired(); }

        SWeakPtr<counter_t::State> state;
    };

    struct RUNTIME_API scheduler_t
    {
        void initialize(const scheudler_config_t&);
        void bind();
        void unbind();
        void shutdown();
        static scheduler_t* instance();
        void schedule(skr_task_t&& task);
        void schedule(eastl::function<void()>&& function);
        struct RUNTIME_API EventAwaitable
        {
            EventAwaitable(scheduler_t& s, event_t event, int workerIdx = -1);
            bool await_ready() const;
            void await_suspend(std::coroutine_handle<skr_task_t::promise_type>);
            void await_resume() const {}
            scheduler_t& scheduler;
            event_t event;
            int workerIdx = -1;
        };
        struct RUNTIME_API CounterAwaitable
        {
            CounterAwaitable(scheduler_t& s, counter_t counter, int workerIdx = -1);
            bool await_ready() const;
            void await_suspend(std::coroutine_handle<skr_task_t::promise_type>);
            void await_resume() const {}
            scheduler_t& scheduler;
            counter_t counter;
            int workerIdx = -1;
        };
        void sync(event_t event);
        void sync(counter_t counter);
        std::array<std::atomic<int>, 8> spinningWorkers;
        std::atomic<unsigned int> nextSpinningWorkerIdx = {0x8000000};
        std::atomic<unsigned int> nextEnqueueIndex = {0};
        std::array<void*, 256> workers;
        void* mainWorker = nullptr;
        scheudler_config_t config;
        bool initialized = false;
        bool binded = false;
    };

    inline void schedule(skr_task_t&& task)
    {
        scheduler_t::instance()->schedule(std::move(task));
    }
    inline void schedule(eastl::function<void()>&& function)
    {
        scheduler_t::instance()->schedule(std::move(function));
    }
    RUNTIME_API scheduler_t::EventAwaitable co_wait(event_t event, bool pinned = false);
    RUNTIME_API scheduler_t::CounterAwaitable co_wait(counter_t counter, bool pinned = false);
    RUNTIME_API void wait(event_t event);
    RUNTIME_API void wait(counter_t counter);
    inline void sync(event_t event)
    {
        scheduler_t::instance()->sync(std::move(event));
    }
    inline void sync(counter_t counter)
    {
        scheduler_t::instance()->sync(std::move(counter));
    }
}
}