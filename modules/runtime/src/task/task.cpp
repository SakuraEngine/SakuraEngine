#include "task/task.hpp"
#include "platform/debug.h"

namespace skr::task
{
#if !defined(SKR_TASK_MARL)
scheudler_config_t::scheudler_config_t()
{
    numThreads = ftl::GetNumHardwareThreads();
}

counter_t::counter_t()
{
    internal = eastl::make_shared<ftl::TaskCounter>(details::get_scheduler()->internal);
}
event_t::event_t()
{
    internal = eastl::make_shared<ftl::TaskCounter>(details::get_scheduler()->internal);
}
thread_local scheduler_t* scheduler = nullptr;
void scheduler_t::initialize(const scheudler_config_t& config)
{
    options.ThreadPoolSize = config.numThreads;
}
void scheduler_t::bind()
{
    SKR_ASSERT(scheduler == nullptr);
    SKR_ASSERT(internal == nullptr);
    internal = new ftl::TaskScheduler();
    options.Callbacks.Context = this;
    options.Callbacks.OnWorkerThreadStarted = [](void* context, unsigned threadIndex)
    {
        scheduler = (scheduler_t*)context;
    };
    options.Callbacks.OnWorkerThreadEnded = [](void* context, unsigned threadIndex)
    {
        scheduler = nullptr;
    };
    options.Callbacks.OnFiberDetached = [](void* context, ftl::Fiber* fiberIndex, bool isMidTask)
    {
        auto s = (scheduler_t*)context;
        for(auto& f : s->onFiberDettached)
        {
            if(!isMidTask)
                f.on_fiber_dettached(fiberIndex);
        }
    };
    internal->Init(options);
    scheduler = this;
}
void scheduler_t::unbind()
{
    SKR_ASSERT(internal);
    SKR_ASSERT(scheduler == this);
    scheduler = nullptr;
    delete internal;
    internal = nullptr;
}
scheduler_t* details::get_scheduler()
{
    SKR_ASSERT(scheduler != nullptr);
    return scheduler;
}
scheduler_t::~scheduler_t()
{
    if(internal)
        delete internal;
}

void* scheduler_t::current_fiber()
{
    return internal->GetCurrentFiber();
}

void* current_fiber()
{
    return details::get_scheduler()->current_fiber();
}
#else
void scheduler_t::initialize(const scheudler_config_t& config)
{
    marl::Scheduler::Config cfg;
    cfg.workerThread.count = config.numThreads;
    internal = new marl::Scheduler(cfg);
}
scheduler_t::~scheduler_t()
{
    if(internal)
        delete internal;
}
#endif
}