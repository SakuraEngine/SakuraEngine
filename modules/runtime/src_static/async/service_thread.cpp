#include "async/service_thread.hpp"
#include "misc/log.h"

namespace skr
{
ServiceThread::ServiceThread(const ServiceThreadDesc& desc) SKR_NOEXCEPT
    : status(kStatusStopped)
{
    f._service = this;

    NamedThreadDesc tDesc = {};
    tDesc.name = desc.name;
    tDesc.priority = SKR_THREAD_NORMAL;
    tDesc.stack_size = 16 * 1024;
    t.initialize(tDesc);
}

ServiceThread::~ServiceThread() SKR_NOEXCEPT
{
    const auto S = get_status();
    if (S != kStatusExitted)
    {
        SKR_LOG_FATAL("service must be exitted before being destroyed!");
        SKR_ASSERT(S == kStatusExitted);
    }

    waitJoin();
    t.finalize();
}

ServiceThread::Status ServiceThread::get_status() const SKR_NOEXCEPT
{
    return static_cast<Status>(skr_atomic32_load_acquire(&status));
}

void ServiceThread::request_stop() SKR_NOEXCEPT
{
    const auto S = get_status();
    if (S != kStatusRunning)
    {
        SKR_LOG_FATAL("must stop from a running service! current status: %d", S);
        SKR_ASSERT(S == kStatusRunning);
    }
    skr_atomic32_store_release(&status, kStatusStopping);
}

void ServiceThread::stop() SKR_NOEXCEPT
{
    request_stop();
    wait_stop();
}

void ServiceThread::wait_stop() SKR_NOEXCEPT
{
    const auto tid = skr_current_thread_id();
    if (tid == t.get_id())
    {
        SKR_LOG_FATAL("dead lock detected!");
        SKR_ASSERT((tid != t.get_id()) && "dead lock detected!");
    }

    while (get_status() != kStatusStopped)
    {
        // ... wait stopping
    }
}

void ServiceThread::run() SKR_NOEXCEPT
{
    const auto S = get_status();
    if (S != kStatusStopped)
    {
        SKR_LOG_FATAL("must run from a stopped service!");
        SKR_ASSERT(S == kStatusStopped);
    }

    skr_atomic32_store_release(&status, kStatusWaking);
    if (!t.has_started())
    {
        t.start(&f);
    }

    while (get_status() != kStatusRunning)
    {
        // ... wait waking
    }
}

void ServiceThread::request_exit() SKR_NOEXCEPT
{
    const auto S = get_status();
    if (S != kStatusStopped)
    {
        SKR_LOG_FATAL("must exit from a stopped service!");
        SKR_ASSERT(S == kStatusStopped);
    }
    skr_atomic32_store_release(&status, kStatusExiting);
}

void ServiceThread::exit() SKR_NOEXCEPT
{
    request_exit();
    wait_exit();
}

void ServiceThread::wait_exit() SKR_NOEXCEPT
{
    const auto tid = skr_current_thread_id();
    if (tid == t.get_id())
    {
        SKR_LOG_FATAL("dead lock detected!");
        SKR_ASSERT((tid != t.get_id()) && "dead lock detected!");
    }

    while (get_status() != kStatusExitted)
    {
        // ... wait stopping
    }
}

void ServiceThread::waitJoin() SKR_NOEXCEPT
{
    wait_exit();
    t.join();
}

AsyncResult ServiceThread::ServiceFunc::run() SKR_NOEXCEPT
{
WAKING:    
{
    auto S = skr_atomic32_load_acquire(&_service->status);
    if (S == kStatusWaking)
    {
        goto RUNNING;
    }
}

RUNNING:
{
    skr_atomic32_store_release(&_service->status, kStatusRunning);
    for (;;)
    {
        // 1. run service
        auto R = _service->serve();
        // 2. check result
        if (R != ASYNC_RESULT_OK)
        {
            // deal_error();
            return R;
        }
        // 3. check status
        auto S = skr_atomic32_load_acquire(&_service->status);
        if (S == kStatusRunning)
        {
            continue;
        }
        else if (S == kStatusStopping)
        {
            goto STOP;
        }
        else // kStatusStopped/Exiting/Exitted/Waking
        {
            SKR_ASSERT(0 && "ServiceThread::serve():RUNNING must set status to kStatusRunning or kStatusStopping");
        }
    }
    SKR_UNREACHABLE_CODE();
    return ASYNC_RESULT_OK;
}

STOP:
{
    auto S = skr_atomic32_load_acquire(&_service->status);
    if (S == kStatusWaking)
    {
        goto WAKING;
    }
    else if (S == kStatusStopped)
    {
        // continue...
    }
    else if (S == kStatusStopping)
    {
        skr_atomic32_store_release(&_service->status, kStatusStopped);
    }
    else if (S == kStatusExiting)
    {
        goto EXIT;
    }
    else // kStatusRunning/Exitted
    {
        SKR_ASSERT(0 && "ServiceThread::serve():STOP must not set status to kStatusRunning or kStatusExitted");
    }
    skr_thread_sleep(1);
    goto STOP;
    SKR_UNREACHABLE_CODE();
}

EXIT:
{
    skr_atomic32_store_release(&_service->status, kStatusExitted);
}
    return ASYNC_RESULT_OK;
}

}