#include "async/async_service.h"
#include "async/wait_timeout.hpp"
#include "misc/log.h"

#include "tracy/Tracy.hpp"

namespace skr
{
ServiceThread::ServiceThread(const ServiceThreadDesc& desc) SKR_NOEXCEPT
    : status(kStatusStopped)
{
    f._service = this;

    NamedThreadDesc tDesc = {};
    tDesc.name = desc.name;
    tDesc.priority = desc.priority;
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
    t.finalize();
}

ServiceThread::Status ServiceThread::get_status() const SKR_NOEXCEPT
{
    return static_cast<Status>(skr_atomic32_load_acquire(&status));
}

void ServiceThread::set_status(Status to_set) SKR_NOEXCEPT
{
    const auto S = get_status();
    if (to_set == kStatusStopping)
    {
        if (S != kStatusRunning)
        {
            SKR_LOG_FATAL("must stop from a running service! current status: %d", S);
            SKR_ASSERT(S == kStatusRunning);  
        }
    }
    else if (to_set == kStatusWaking)
    {
        if (S != kStatusStopped)
        {
            SKR_LOG_FATAL("must wake from a stopped service!");
            SKR_ASSERT(S == kStatusStopped);
        }
    }
    else if (to_set == kStatusExiting)
    {
        if (S != kStatusStopped)
        {
            SKR_LOG_FATAL("must exit from a stopped service!");
            SKR_ASSERT(S == kStatusStopped);
        }
    }
    skr_atomic32_store_release(&status, to_set);
}

void ServiceThread::request_stop() SKR_NOEXCEPT
{
    set_status(kStatusStopping);
}

void ServiceThread::stop() SKR_NOEXCEPT
{
    ZoneScopedN("stop");

    request_stop();
    wait_stop();
}

void ServiceThread::wait_stop(uint32_t fatal_timeout) SKR_NOEXCEPT
{
    ZoneScopedN("wait_stop");

    const auto tid = skr_current_thread_id();
    if (tid == t.get_id())
    {
        SKR_LOG_FATAL("dead lock detected!");
        SKR_ASSERT((tid != t.get_id()) && "dead lock detected!");
    }

    wait_timeout([&] { return get_status() == kStatusStopped; }, fatal_timeout);
}

void ServiceThread::run() SKR_NOEXCEPT
{
    // record last turn id
    const auto orid = skr_atomic32_load_relaxed(&rid);
    
    // signal waking
    set_status(kStatusWaking);
    if (!t.has_started())
    {
        t.start(&f);
    }

    // secure runned
    wait_timeout([&] { return skr_atomic32_load_relaxed(&rid) > orid; }, 8);
}

void ServiceThread::request_exit() SKR_NOEXCEPT
{
    set_status(kStatusExiting);
}

void ServiceThread::exit() SKR_NOEXCEPT
{
    ZoneScopedN("exit");

    // SKR_LOG_TRACE("ServiceThread::destroy: wait runner thread to request_exit...");
    request_exit();
    // SKR_LOG_TRACE("ServiceThread::destroy: wait runner thread to wait_exit...");
    wait_exit();
}

void ServiceThread::wait_exit(uint32_t fatal_timeout) SKR_NOEXCEPT
{
    ZoneScopedN("wait_exit");

    const auto tid = skr_current_thread_id();
    if (tid == t.get_id())
    {
        SKR_LOG_FATAL("dead lock detected!");
        SKR_ASSERT((tid != t.get_id()) && "dead lock detected!");
    }

    const auto S = get_status();
    if (S < kStatusExiting)
    {
        SKR_LOG_FATAL("must wait from a exiting service!");
        SKR_ASSERT(S  < kStatusStopped);
    }
    
    wait_timeout([&] { return get_status() == kStatusExitted; }, fatal_timeout);
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
    ZoneScopedN("WAKING");
    auto S = _service->get_status();
    if (S == kStatusWaking)
    {
        goto RUNNING;
    }
}

RUNNING:
{
    ZoneScopedN("RUNNING");
    _service->set_status(kStatusRunning);
    skr_atomic32_add_relaxed(&_service->rid, 1);
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
        auto S = _service->get_status();
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
    ZoneScopedN("STOP");

    auto S = _service->get_status();
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
        _service->set_status(kStatusStopped);
    }
    else if (S == kStatusExiting)
    {
        goto EXIT;
    }
    else // kStatusRunning/Exitted
    {
        SKR_ASSERT(0 && "ServiceThread::serve():STOP must not set status to kStatusRunning or kStatusExitted");
    }
    skr_thread_sleep(0);
    goto STOP;
    SKR_UNREACHABLE_CODE();
}

EXIT:
{
    ZoneScopedN("EXIT");

    _service->set_status(kStatusExitted);
    // SKR_LOG_TRACE("Service Thread exited!");
}
    return ASYNC_RESULT_OK;
}

void AsyncService::sleep() SKR_NOEXCEPT
{
    const auto ms = skr_atomicu32_load_relaxed(&sleep_time);
    ZoneScopedNC("ioServiceSleep(Cond)", tracy::Color::Gray55);

    condlock.lock();
    if (!event)
    {
        condlock.wait(ms);
    }
    event = false;
    condlock.unlock();
}

}