#pragma once
#include "SkrRT/async/thread_job.hpp"
#include "SkrRT/async/named_thread.hpp"
#include "SkrRT/async/condlock.hpp"

namespace skr
{
using JobQueueThreadDesc = NamedThreadDesc;
struct JobThreadFunction : public NamedThreadFunction { virtual ~JobThreadFunction() {} };
struct JobQueueThread : public NamedThread 
{

};

using JobQueueCondDesc = CondLockDesc;
using JobQueueCond = CondLock;

} // namespace skr