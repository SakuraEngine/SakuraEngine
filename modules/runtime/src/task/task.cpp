#include "task/task.hpp"
#include "platform/debug.h"

namespace skr::task
{
#if !SKR_TASK_MARL
counter_t::counter_t()
{
    internal = std::make_shared<internal_t>(details::get_scheduler()->internal);
}
event_t::event_t()
{
    internal = std::make_shared<internal_t>(details::get_scheduler()->internal);
}
thread_local scheduler_t* scheduler = nullptr;
void scheduler_t::initialize(const scheudler_config_t& config)
{
    options.ThreadPoolSize = config.numThreads;
}
void scheduler_t::bind()
{
    internal = new ftl::TaskScheduler();
    internal->Init(options);
    scheduler = this;
    binded = true;
}
void scheduler_t::unbind()
{
    SKR_ASSERT(binded);
    SKR_ASSERT(scheduler == this);
    scheduler = nullptr;
    binded = false;
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