#pragma once
#include "SkrCore/async/thread_job.hpp"
#include "SkrCore/async/named_thread.hpp"
#include "SkrCore/async/condlock.hpp"

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