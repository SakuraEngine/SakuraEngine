#pragma once
#include "utils/defer.hpp"
#include "EASTL/shared_ptr.h"
#include "runtime_module.h"
#include "EASTL/functional.h"
#include "platform/thread.h"

namespace skr::task
{
    class counter_t;
    class event_t;
    class scheduler_t;
    struct RUNTIME_API scheudler_config_t
    {
        scheudler_config_t();
        uint32_t numThreads = 0;
    };
    struct fiber_listener_t
    {
        template<typename T>
        fiber_listener_t(T* t)
        {
            self = t;
            on_fiber_dettached_ptr = +[](void* self, void* fiber)
            {
                (*reinterpret_cast<T*>(self)).on_fiber_dettached(fiber);
            };
        }
        void on_fiber_dettached(void* fiber)
        {
            on_fiber_dettached_ptr(self, fiber);
        }
        void* self;
    private:
        void(*on_fiber_dettached_ptr)(void* self, void* fiber) = nullptr;
    };
}
#define SKR_TASK_MARL
#if !defined(SKR_TASK_MARL)
#include "ftl/task_scheduler.h"
#include "ftl/task_counter.h"
namespace skr::task
{
    class RUNTIME_API counter_t
    {
    public:
        using internal_t = eastl::shared_ptr<ftl::TaskCounter>;
        counter_t();
        counter_t(std::nullptr_t) {}

        bool operator==(const counter_t& other) const { return internal == other.internal; }
        void wait(bool pin) { internal->GetScheduler()->WaitForCounter(internal.get(), pin); }
        void add(const unsigned int x) { internal->Add(x); }
        void decrement() { internal->Decrement(); }
        size_t hash() const { return std::hash<void*>{}(internal.get()); }
        explicit operator bool() const { return (bool)internal; }
    private:
        internal_t internal;
        friend scheduler_t;
    };

    class RUNTIME_API event_t
    {
    public:
        using internal_t = eastl::shared_ptr<ftl::TaskCounter>;
        event_t();
        event_t(std::nullptr_t) {}
        bool operator==(const event_t& other) const { return internal == other.internal; }
        void wait(bool pin) { internal->GetScheduler()->WaitForCounter(internal.get(), pin); }
        void signal() { internal->Decrement(); }
        void clear() { internal->Reset(1); }
        bool test() const { return internal->Done(); }
        size_t hash() const { return std::hash<void*>{}(internal.get()); }
        explicit operator bool() const { return (bool)internal; }
        
    private:
        internal_t internal;
        friend scheduler_t;
    };

    class RUNTIME_API scheduler_t
    {
        using internal_t = ftl::TaskScheduler*;
    public:
        void initialize(const scheudler_config_t&);
        void bind();
        void unbind();
        template<class F>
        void schedule(F&& lambda, event_t* event, const char* name = nullptr)
        {
            auto f = SkrNewLambda(std::forward<F>(lambda));
            using f_t = decltype(f);
            auto t = +[](ftl::TaskScheduler* taskScheduler, void* data)
            {
                auto f = (f_t)data;
                SKR_DEFER({SkrDelete(f);});
                f->operator()(); 
            };
            ftl::Task task{t, f};
            internal->AddTask(task, ftl::TaskPriority::Normal, event ? event->internal : nullptr, name);
        }
        void* current_fiber(); 
        ~scheduler_t();
        SMutexObject onFiberDettachedMutex;
        eastl::vector<fiber_listener_t> onFiberDettached;
    private:
        internal_t internal = nullptr;
        ftl::TaskSchedulerInitOptions options;
        friend class counter_t;
        friend class event_t;
    };

    template<class F>
    void schedule(F&& lambda, event_t* event, const char* name = nullptr);

    void* current_fiber();

    struct RUNTIME_API details
    {
        private:
        static scheduler_t* get_scheduler();
        template<class F>
        friend void on_fiber_dettached(F&& f);
        friend class counter_t;
        friend class event_t;
        template<class F>
        friend void schedule(F&& lambda, event_t* event, const char* name);
        friend void* current_fiber();
    };

    template<class F>
    void on_fiber_dettached(F&& f)
    {
        auto scheduler = details::get_scheduler();
        SMutexLock lock(scheduler->onFiberDettachedMutex.mMutex);
        std::forward<F>(f)(scheduler->onFiberDettached);
    }

    template<class F>
    void schedule(F&& lambda, event_t* event, const char* name)
    {
        scheduler_t* scheduler = details::get_scheduler();
        SKR_ASSERT(scheduler);
        scheduler->schedule(std::forward<F>(lambda), event, name);
    }
}
#else
#include "marl/scheduler.h"
#include "marl/event.h"
#include "marl/waitgroup.h"

namespace skr::task
{
    class RUNTIME_API counter_t
    {
    public:
        using internal_t = marl::WaitGroup;
        counter_t() = default;
        counter_t(std::nullptr_t) : internal(nullptr) {}

        bool operator==(const counter_t& other) const { return internal == other.internal; }
        size_t hash() const { return internal.hash(); }
        explicit operator bool() const { return (bool)internal; }
        void wait(bool pin) { internal.wait(); }
        void add(const uint32_t x) { internal.add(x); }
        void decrement() { internal.done(); }
    private:
        internal_t internal;
    };

    class RUNTIME_API event_t
    {
    public:
        using internal_t = marl::Event;
        event_t() : internal(marl::Event::Mode::Manual) {}
        event_t(std::nullptr_t) : internal(nullptr) {}

        bool operator==(const event_t& other) const { return internal == other.internal; }
        void wait(bool pin) { internal.wait(); }
        void signal() { internal.signal(); }
        void clear() { internal.clear(); }
        bool test() const { return internal.test(); }
        size_t hash() const { return internal.hash(); }
        explicit operator bool() const { return (bool)internal; }
    private:
        internal_t internal;
    };

    class RUNTIME_API scheduler_t
    {
    public:
        using internal_t = marl::Scheduler*;
        void initialize(const scheudler_config_t&);
        void bind() { internal->bind(); }
        void unbind() { internal->unbind(); }
        ~scheduler_t();
    private:
        internal_t internal = nullptr;
    };

    inline void* current_fiber() { return marl::Scheduler::Fiber::current(); }

    template<class F>
    void schedule(F&& lambda, event_t* event, const char* name = nullptr)
    {
        //TODO: trace name
        if(event && *event)
        {
            marl::schedule([event = *event, lambda = std::forward<F>(lambda)]() mutable
            {
                SKR_DEFER({ event.signal(); });
                lambda();
            });
        }
        else
        {
            marl::schedule([lambda = std::forward<F>(lambda)]() mutable
            {
                lambda();
            });
        }
    }
}

#endif

namespace skr::task
{
    
}