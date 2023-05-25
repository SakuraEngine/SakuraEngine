#pragma once
#include "async/thread_job.hpp"
#include "async/named_thread.hpp"
#include "async/condlock.hpp"

namespace skr
{
using JobQueueThreadDesc = NamedThreadDesc;
struct JobThreadFunction : public NamedThreadFunction { virtual ~JobThreadFunction() {} };
struct JobQueueThread : public NamedThread 
{
    JobQueueThread(NamedThreadPriority priority, 
        uint32_t stackSize, const NamedThreadDesc *desc = nullptr) SKR_NOEXCEPT
        : NamedThread(priority, stackSize, desc) {}
};

using JobQueueCondDesc = CondLockDesc;
using JobQueueCond = CondLock;

} // namespace skr