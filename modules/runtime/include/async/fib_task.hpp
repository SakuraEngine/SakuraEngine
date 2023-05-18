#pragma once
#include "platform/configure.h"
#include "utils/defer.hpp"
#include "platform/debug.h"

namespace skr::task
{
    struct counter_t;
    struct event_t;
    struct weak_counter_t;
    struct weak_event_t;
    struct scheduler_t;
    struct RUNTIME_API scheudler_config_t
    {
        scheudler_config_t();
        uint32_t numThreads = 0;
    };

    template<class F>
    void wait(bool pin, F&& pred);
}

#define SKR_TASK_MARL

#if !defined(SKR_TASK_MARL)
#include "ftl/task_scheduler.h"
#include "ftl/task_counter.h"
#include "EASTL/shared_ptr.h"

namespace skr::task
{
    struct RUNTIME_API counter_t
    {
    public:
        using internal_t = eastl::shared_ptr<ftl::TaskCounter>;
        counter_t(bool inverse = false);
        counter_t(std::nullptr_t) {}

        bool operator==(const counter_t& other) const { return internal == other.internal; }
        void wait(bool pin) const { internal->GetScheduler()->WaitForCounter(internal.get(), pin); }
        void add(const unsigned int x) { internal->Add(x); }
        void decrement() { internal->Decrement(); }
        bool test() const { return internal->Done(); }
        size_t hash() const { return std::hash<void*>{}(internal.get()); }
        explicit operator bool() const { return (bool)internal; }
    private:
        friend struct weak_counter_t;
        counter_t(internal_t&& internal) : internal(std::move(internal)) {}
        internal_t internal;
        friend scheduler_t;
    };

    struct RUNTIME_API weak_counter_t
    {
    public:
        using internal_t = eastl::weak_ptr<ftl::TaskCounter>;
        weak_counter_t() = default;
        weak_counter_t(const counter_t& counter) { internal = counter.internal; }
        weak_counter_t(const weak_counter_t& other) { internal = other.internal; }
        weak_counter_t(weak_counter_t&& other) { internal = std::move(other.internal); }
        weak_counter_t& operator=(const weak_counter_t& other) { internal = other.internal; return *this; }
        weak_counter_t& operator=(weak_counter_t&& other) { internal = std::move(other.internal); return *this; }
        bool operator==(const weak_counter_t& other) const { return internal.lock() == other.internal.lock(); }
        counter_t lock() const { return counter_t(internal.lock()); }
        bool expired() const { return internal.expired(); }
    private:
        internal_t internal;
        friend scheduler_t;
    };

    struct RUNTIME_API event_t
    {
    public:
        using internal_t = eastl::shared_ptr<ftl::TaskCounter>;
        event_t();
        event_t(std::nullptr_t) {}
        bool operator==(const event_t& other) const { return internal == other.internal; }
        void wait(bool pin) const { internal->GetScheduler()->WaitForCounter(internal.get(), pin); }
        void signal() { internal->Decrement(); }
        void clear() { internal->Reset(1); }
        bool test() const { return internal->Done(); }
        size_t hash() const { return std::hash<void*>{}(internal.get()); }
        explicit operator bool() const { return (bool)internal; }
        
    private:
        friend struct weak_event_t;
        event_t(internal_t&& internal) : internal(std::move(internal)) {}
        internal_t internal;
        friend scheduler_t;
    };

    struct RUNTIME_API weak_event_t
    {
    public:
        using internal_t = eastl::weak_ptr<ftl::TaskCounter>;
        weak_event_t() = default;
        weak_event_t(const event_t& event) { internal = event.internal; }
        weak_event_t(const weak_event_t& other) { internal = other.internal; }
        weak_event_t(weak_event_t&& other) { internal = std::move(other.internal); }
        weak_event_t& operator=(const weak_event_t& other) { internal = other.internal; return *this; }
        weak_event_t& operator=(weak_event_t&& other) { internal = std::move(other.internal); return *this; }
        bool operator==(const weak_event_t& other) const { return internal.lock() == other.internal.lock(); }
        event_t lock() const { return event_t(internal.lock()); }
        bool expired() const { return internal.expired(); }
    private:
        internal_t internal;
        friend scheduler_t;
    };

    struct RUNTIME_API scheduler_t
    {
        using internal_t = ftl::TaskScheduler*;
    public:
        void initialize(const scheudler_config_t&);
        void bind();
        void unbind();
        template<struct F>
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
            internal->AddTask(task, ftl::TaskPriority::Normal, event ? event->internal : nullptr FTL_TASK_NAME(, name));
            if(event) event->internal->Decrement();
        }
        
        template<struct F>
        void wait(bool pin, F&& lambda)
        {
            internal->WaitForPredicate(std::forward<F>(lambda), pin);
        }
        void* current_fiber(); 
        ~scheduler_t();
    private:
        internal_t internal = nullptr;
        ftl::TaskSchedulerInitOptions options;
        friend struct counter_t;
        friend struct event_t;
    };

    template<struct F>
    void schedule(F&& lambda, event_t* event, const char* name = nullptr);

    void* current_fiber();

    struct RUNTIME_API details
    {
        private:
        static scheduler_t* get_scheduler();
        friend struct counter_t;
        friend struct event_t;
        template<struct F>
        friend void schedule(F&& lambda, event_t* event, const char* name);
        template<struct F>
        friend void wait(bool pin, F&& lambda);
        friend void* current_fiber();
    };

    template<struct F>
    void schedule(F&& lambda, event_t* event, const char* name)
    {
        scheduler_t* scheduler = details::get_scheduler();
        SKR_ASSERT(scheduler);
        scheduler->schedule(std::forward<F>(lambda), event, name);
    }

    template<struct F>
    void wait(bool pin, F&& lambda)
    {
        scheduler_t* scheduler = details::get_scheduler();
        SKR_ASSERT(scheduler);
        scheduler->wait(pin, std::forward<F>(lambda));
    }
}
#else
#include "marl/event.h"
#include "marl/waitgroup.h"

namespace skr::task
{
    struct RUNTIME_API counter_t
    {
    public:
        using internal_t = marl::WaitGroup;
        counter_t(bool inverse = false) : internal(0, inverse) {}
        counter_t(std::nullptr_t) : internal(nullptr) {}

        bool operator==(const counter_t& other) const { return internal == other.internal; }
        size_t hash() const { return internal.hash(); }
        explicit operator bool() const { return (bool)internal; }
        void wait(bool pin) const { internal.wait(); }
        void add(const uint32_t x) { internal.add(x); }
        bool test() const { return internal.test(); }
        void decrement() { internal.done(); }
    private:
        friend struct weak_counter_t;
        counter_t(internal_t&& other) : internal(std::move(other)) {}
        internal_t internal;
    };

    struct RUNTIME_API weak_counter_t
    {
    public:
        using internal_t = marl::WeakWaitGroup;
        weak_counter_t() = default;
        weak_counter_t(std::nullptr_t) : internal(nullptr) {}
        weak_counter_t(const counter_t& counter) : internal(counter.internal) {}
        weak_counter_t(const weak_counter_t& other) : internal(other.internal) {}
        bool operator==(const weak_counter_t& other) const { return internal == other.internal; }
        counter_t lock() const { return internal.lock(); }
        bool expired() const { return internal.expired(); }
        
    private:
        internal_t internal;
    };

    struct RUNTIME_API event_t
    {
    public:
        using internal_t = marl::Event;
        event_t() : internal(marl::Event::Mode::Manual) {}
        event_t(std::nullptr_t) : internal(nullptr) {}

        bool operator==(const event_t& other) const { return internal == other.internal; }
        void wait(bool pin) const 
        {
            internal.wait();
            //if (pin)
            //{
            //    bool finished = false;
            //    while (!finished) finished = internal.test();
            //}
            //else
            //{
            //    internal.wait();
            //}
        }
        void signal() { internal.signal(); }
        void clear() { internal.clear(); }
        bool test() const { return internal.test(); }
        size_t hash() const { return internal.hash(); }
        explicit operator bool() const { return (bool)internal; }
    private:
        friend struct weak_event_t;
        event_t(internal_t&& other) : internal(std::move(other)) {}
        internal_t internal;
    };

    struct RUNTIME_API weak_event_t
    {
    public:
        using internal_t = marl::WeakEvent;
        weak_event_t() = default;
        weak_event_t(std::nullptr_t) : internal(nullptr) {}
        weak_event_t(const event_t& event) : internal(event.internal) {}
        weak_event_t(const weak_event_t& other) : internal(other.internal) {}
        bool operator==(const weak_event_t& other) const { return internal == other.internal; }
        event_t lock() const { return internal.lock(); }
        bool expired() const { return internal.expired(); }
    private:
        internal_t internal;
    };

    struct RUNTIME_API scheduler_t
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

    template<class F>
    void wait(bool pin, F&& lambda)
    {
        SKR_UNIMPLEMENTED_FUNCTION();
        using namespace std::chrono_literals;
        auto fiber = marl::Scheduler::Fiber::current();
        SKR_ASSERT(fiber);
        marl::mutex mutex;
        marl::lock lock(mutex);
        const std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
        fiber->wait(lock, now + 0.01ms, lambda);
    }
}

#endif
