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
    class weak_counter_t;
    class weak_event_t;
    class scheduler_t;
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
namespace skr::task
{
    class RUNTIME_API counter_t
    {
    public:
        using internal_t = eastl::shared_ptr<ftl::TaskCounter>;
        counter_t();
        counter_t(std::nullptr_t) {}

        bool operator==(const counter_t& other) const { return internal == other.internal; }
        void wait(bool pin) const { internal->GetScheduler()->WaitForCounter(internal.get(), pin); }
        void add(const unsigned int x) { internal->Add(x); }
        void decrement() { internal->Decrement(); }
        bool test() const { return internal->Done(); }
        size_t hash() const { return std::hash<void*>{}(internal.get()); }
        explicit operator bool() const { return (bool)internal; }
    private:
        friend class weak_counter_t;
        counter_t(internal_t&& internal) : internal(std::move(internal)) {}
        internal_t internal;
        friend scheduler_t;
    };

    class RUNTIME_API weak_counter_t
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
        size_t hash() const { if (auto ptr = internal.lock()) return std::hash<void*>{}(ptr.get()); else return 0; }
        counter_t lock() const { return counter_t(internal.lock()); }
        bool expired() const { return internal.expired(); }
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
        void wait(bool pin) const { internal->GetScheduler()->WaitForCounter(internal.get(), pin); }
        void signal() { internal->Decrement(); }
        void clear() { internal->Reset(1); }
        bool test() const { return internal->Done(); }
        size_t hash() const { return std::hash<void*>{}(internal.get()); }
        explicit operator bool() const { return (bool)internal; }
        
    private:
        friend class weak_event_t;
        event_t(internal_t&& internal) : internal(std::move(internal)) {}
        internal_t internal;
        friend scheduler_t;
    };

    class RUNTIME_API weak_event_t
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
        size_t hash() const { if (auto ptr = internal.lock()) return std::hash<void*>{}(ptr.get()); else return 0; }
        event_t lock() const { return event_t(internal.lock()); }
        bool expired() const { return internal.expired(); }
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
            internal->AddTask(task, ftl::TaskPriority::Normal, event ? event->internal : nullptr FTL_TASK_NAME(, name));
        }
        
        template<class F>
        void wait(bool pin, F&& lambda)
        {
            internal->WaitForPredicate(std::forward<F>(lambda), pin);
        }
        void* current_fiber(); 
        ~scheduler_t();
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
        friend class counter_t;
        friend class event_t;
        template<class F>
        friend void schedule(F&& lambda, event_t* event, const char* name);
        template<class F>
        friend void wait(bool pin, F&& lambda);
        friend void* current_fiber();
    };

    template<class F>
    void schedule(F&& lambda, event_t* event, const char* name)
    {
        scheduler_t* scheduler = details::get_scheduler();
        SKR_ASSERT(scheduler);
        scheduler->schedule(std::forward<F>(lambda), event, name);
    }

    template<class F>
    void wait(bool pin, F&& lambda)
    {
        scheduler_t* scheduler = details::get_scheduler();
        SKR_ASSERT(scheduler);
        scheduler->wait(pin, std::forward<F>(lambda));
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
        void wait(bool pin) const { internal.wait(); }
        void add(const uint32_t x) { internal.add(x); }
        bool test() const { return internal.test(); }
        void decrement() { internal.done(); }
    private:
        friend class weak_counter_t;
        counter_t(internal_t&& other) : internal(std::move(other)) {}
        internal_t internal;
    };

    class RUNTIME_API weak_counter_t
    {
    public:
        using internal_t = marl::WeakWaitGroup;
        weak_counter_t() = default;
        weak_counter_t(std::nullptr_t) : internal(nullptr) {}
        weak_counter_t(const counter_t& counter) : internal(counter.internal) {}
        weak_counter_t(const weak_counter_t& other) : internal(other.internal) {}
        bool operator==(const weak_counter_t& other) const { return internal == other.internal; }
        size_t hash() const { return internal.hash(); }
        counter_t lock() const { return internal.lock(); }
        bool expired() const { return internal.expired(); }
        
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
        void wait(bool pin) const { internal.wait(); }
        void signal() { internal.signal(); }
        void clear() { internal.clear(); }
        bool test() const { return internal.test(); }
        size_t hash() const { return internal.hash(); }
        explicit operator bool() const { return (bool)internal; }
    private:
        friend class weak_event_t;
        event_t(internal_t&& other) : internal(std::move(other)) {}
        internal_t internal;
    };

    class RUNTIME_API weak_event_t
    {
    public:
        using internal_t = marl::WeakEvent;
        weak_event_t() = default;
        weak_event_t(std::nullptr_t) : internal(nullptr) {}
        weak_event_t(const event_t& event) : internal(event.internal) {}
        weak_event_t(const weak_event_t& other) : internal(other.internal) {}
        bool operator==(const weak_event_t& other) const { return internal == other.internal; }
        size_t hash() const { return internal.hash(); }
        event_t lock() const { return internal.lock(); }
        bool expired() const { return internal.expired(); }
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

    template<class F>
    void wait(bool pin, F&& lambda)
    {
        auto fiber = marl::Scheduler::Fiber::current();
        SKR_ASSERT(fiber);
        marl::mutex mutex;
        marl::lock lock(mutex);
        fiber->wait(lock, lambda);
    }
}

#endif

namespace skr::task
{
    
}