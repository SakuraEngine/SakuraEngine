#include "SkrTask/fib_task.hpp"
#include "SkrBase/misc/debug.h" 

namespace skr::task
{
#if !defined(SKR_TASK_MARL)
scheudler_config_t::scheudler_config_t()
{
    numThreads = ftl::GetNumHardwareThreads();
}

counter_t::counter_t(bool inverse)
{
    internal = skr::make_shared<ftl::TaskCounter>(nullptr, inverse);
}

event_t::event_t()
{
    internal = skr::make_shared<ftl::TaskCounter>(nullptr);
    internal->Add(1);
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
        ftl::BindScheduler(scheduler->internal);
    };
    options.Callbacks.OnWorkerThreadEnded = [](void* context, unsigned threadIndex)
    {
        scheduler = nullptr;
        ftl::UnbindScheduler();
    };
    internal->Init(options);
    scheduler = this;
    ftl::BindScheduler(internal);
}
void scheduler_t::unbind()
{
    SKR_ASSERT(internal);
    SKR_ASSERT(scheduler == this);
    scheduler = nullptr;
    delete internal;
    internal = nullptr;
    ftl::UnbindScheduler();
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
    if (!cfg.workerThread.count)
    {
        cfg.allCores();
    }
    internal = new marl::Scheduler(cfg);
}
scheudler_config_t::scheudler_config_t()
{
    numThreads = marl::Thread::numLogicalCPUs();
}
scheduler_t::~scheduler_t()
{
    if(internal)
        delete internal;
}
#endif
}