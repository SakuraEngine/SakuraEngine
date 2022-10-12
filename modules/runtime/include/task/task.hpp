#pragma once
#include "utils/defer.hpp"

namespace skr::task
{
    class counter_t;
    class event_t;
    class scheduler_t;
    struct scheudler_config_t
    {
        scheudler_config_t();
        uint32_t numThreads = 0;
    };
}
//#define SKR_TASK_MARL
#if !SKR_TASK_MARL
#include "ftl/task_scheduler.h"
#include "ftl/task_counter.h"
namespace skr::task
{
    class counter_t
    {
    public:
        using internal_t = std::shared_ptr<ftl::TaskCounter>;
        counter_t();

        void wait(bool pin) { internal->GetScheduler()->WaitForCounter(internal.get(), pin); }
        void add(const unsigned int x) { internal->Add(x); }
        void decrement() { internal->Decrement(); }
    private:
        internal_t internal;
        friend scheduler_t;
    };

    class event_t
    {
    public:
        using internal_t = std::shared_ptr<ftl::TaskCounter>;
        event_t();
        void wait(bool pin) { internal->GetScheduler()->WaitForCounter(internal.get(), pin); }
        void signal() { internal->Decrement(); }
        void clear() { internal->Reset(1); }
        bool test() const { return internal->Done(); }
    private:
        static size_t gId;
        size_t id;
        internal_t internal;
        friend scheduler_t;
    };

    class scheduler_t
    {
        using internal_t = ftl::TaskScheduler*;
    public:
        void initialize(const scheudler_config_t&);
        void bind();
        void unbind();
        template<class F>
        void schedule(F&& lambda, event_t* event, const char* name = nullptr)
        {
            auto f = new auto(std::forward<F>(lambda));
            using f_t = decltype(f);
            auto t = +[](ftl::TaskScheduler* taskScheduler, void* data)
            {
                auto f = (f_t)data;
                f->operator()(); 
            };
            auto clear = ftl::PostTask([f] { delete f; });
            ftl::Task task{t, f, clear};
            internal->AddTask(task, ftl::TaskPriority::Normal, event ? event->internal : nullptr, name);
        }
        ~scheduler_t();

    private:
        internal_t internal = nullptr;
        ftl::TaskSchedulerInitOptions options;
        bool binded = false;
        friend class counter_t;
        friend class event_t;
    };

    template<class F>
    void schedule(F&& lambda, event_t* event, const char* name = nullptr);

    struct details
    {
        private:
        static scheduler_t* get_scheduler();
        friend class counter_t;
        friend class event_t;
        template<class F>
        friend void schedule(F&& lambda, event_t* event, const char* name);
    };

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
    class counter_t
    {
    public:
        using internal_t = marl::WaitGroup;
        void wait(bool pin) { internal.wait(); }
        void add(const unsigned int x) { internal.add(x); }
        void decrement() { internal.done(); }
    private:
        internal_t internal;
    };

    class event_t
    {
    public:
        using internal_t = marl::Event;
        void wait(bool pin) { internal.wait(); }
        void signal() { internal.signal(); }
        void clear() { internal.clear(); }
        bool test() const { return internal.test(); }
    private:
        internal_t internal;
    };

    class scheduler_t
    {
        using internal_t = marl::Scheduler*;
        void initialize(const scheudler_config_t&);
        void bind() { internal->bind(); }
        void unbind() { internal->unbind(); }
        ~scheduler_t();
    private:
        internal_t internal = nullptr;
    };

    template<class F>
    void schedule(F&& lambda, event_t* event)
    {
        if(event)
        {
            marl::schedule([event = *event, lambda]() mutable
            {
                SKR_DEFER({ event.signal(); });
                lambda();
            });
        }
        else
        {
            marl::schedule([lambda]
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