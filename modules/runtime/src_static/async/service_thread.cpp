#include "SkrRT/misc/log.h"
#include "SkrRT/async/async_service.h"
#include "SkrRT/async/wait_timeout.hpp"
#include "SkrBase/misc/defer.hpp"

#include "SkrProfile/profile.h"

namespace skr
{
ServiceThread::ServiceThread(const ServiceThreadDesc& desc) SKR_NOEXCEPT
    : status_(kStatusStopped)
{
    f._service = this;

    NamedThreadDesc tDesc = {};
    tDesc.name            = desc.name;
    tDesc.priority        = desc.priority;
    tDesc.stack_size      = 16 * 1024;
    t.initialize(tDesc);
}

ServiceThread::~ServiceThread() SKR_NOEXCEPT
{
    const auto S = get_status();
    if (S != kStatusExitted)
    {
        SKR_LOG_FATAL(u8"service must be exitted before being destroyed!");
        SKR_ASSERT(S == kStatusExitted);
    }
    t.finalize();
}

ServiceThread::Status ServiceThread::get_status() const SKR_NOEXCEPT
{
    return static_cast<Status>(skr_atomic32_load_acquire(&status_));
}

ServiceThread::Action ServiceThread::get_action() const SKR_NOEXCEPT
{
    return static_cast<Action>(skr_atomic32_load_acquire(&action_));
}

uint64_t ServiceThread::request(Action action) SKR_NOEXCEPT
{
    // wait last event
    wait_timeout<u8"WaitLastAction">([&] { return get_action() == kActionNone; });

    auto eid = skr_atomicu64_add_relaxed(&event_, 1) + 1;
    setAction(action);
    return eid;
}

void ServiceThread::wait(uint64_t event, uint32_t fatal_timeout) SKR_NOEXCEPT
{
    auto now = skr_atomicu64_load_acquire(&event_);
    if (now == event)
        wait_timeout<u8"WaitLastEvent">([&]{ return get_action() == kActionNone; }, fatal_timeout);
    else if (now > event)
        return;
    else if (now < event)
        SKR_LOG_FATAL(u8"service_thread: can't wait an event that doesn't exist! current event: %llu, target event: %llu", now, event);
}

void ServiceThread::stop() SKR_NOEXCEPT
{
    SkrZoneScopedN("stop");
    auto e = request_stop();
    wait(e);
}

void ServiceThread::run() SKR_NOEXCEPT
{
    SkrZoneScopedN("run");
    auto e = request_run();
    wait(e);
}

void ServiceThread::exit() SKR_NOEXCEPT
{
    SkrZoneScopedN("exit");
    auto e = request_exit();
    wait(e);

    t.finalize();
}

void ServiceThread::setAction(Action target) SKR_NOEXCEPT
{
    skr_atomic32_store_release(&action_, target);
    SKR_LOG_BACKTRACE(u8"service_thread: action set to: %d.", target);
}

void ServiceThread::setStatus(Status target) SKR_NOEXCEPT
{
    const auto S = get_status();
    if (S == target)
        return;

    if ((target == kStatusStopped) && (S == kStatusExitted))
    {
        SKR_LOG_FATAL(u8"service_thread: must stop from a running/stopped service! current status: %d", S);
        SKR_ASSERT(S != kStatusExitted);
    }
    else if ((target == kStatusRunning) && (S != kStatusStopped))
    {
        SKR_LOG_FATAL(u8"service_thread: must wake from a stopped service! current status: %d", S);
        SKR_ASSERT(S == kStatusStopped);
    }
    else if ((target == kStatusExitted) && (S != kStatusStopped))
    {
        SKR_LOG_FATAL(u8"service_thread: must exit from a stopped service! current status: %d", S);
        SKR_ASSERT(S == kStatusStopped);
    }
    skr_atomic32_store_release(&status_, target);
    SKR_LOG_BACKTRACE(u8"service_thread: status set to: %d.", target);
}

void ServiceThread::waitStatus(Status target, uint32_t fatal_timeout) SKR_NOEXCEPT
{
    SkrZoneScopedN("waitStatus");

    const auto tid = skr_current_thread_id();
    if (tid == t.get_id())
    {
        SKR_LOG_FATAL(u8"service_thread: dead lock detected!");
        SKR_ASSERT((tid != t.get_id()) && "service_thread: dead lock detected!");
    }

    const auto current = get_status();
    if (const auto earlyCheck = (current == target))
        return;

    if ((target == kStatusRunning) && (current != kStatusStopped))
    {
        SKR_LOG_FATAL(u8"service_thread: must wait from a waking service!(current status: %d)", current);
        SKR_ASSERT(current == kStatusStopped);
    }
    else if ((target == kStatusStopped) && (current != kStatusRunning))
    {
        SKR_LOG_FATAL(u8"service_thread: must wait from a stopping service!(current status: %d)", current);
        SKR_ASSERT(current == kStatusRunning);
    }
    else if ((target == kStatusExitted) && (current != kStatusStopped))
    {
        SKR_LOG_FATAL(u8"service_thread: must wait from a exiting service!(current status: %d)", current);
        SKR_ASSERT(current == kStatusStopped);
    }

    if (!wait_timeout<u8"WaitStatus">([&] { return get_status() == target; }, fatal_timeout))
        SKR_LOG_FATAL(u8"service_thread: waitStatus timeout! current status: %d, target status: %d", current, target);
}

ServiceThread::Status ServiceThread::takeAction() SKR_NOEXCEPT
{
    const auto act = get_action();
    if (act != kActionNone)
    {
        const Status lut[] = { kStatusRunning, kStatusStopped, kStatusExitted };
        const auto   out   = lut[act - 1];
        setStatus(out);
        setAction(kActionNone);
        return out;
    }
    return get_status();
}

AsyncResult ServiceThread::ServiceFunc::run() SKR_NOEXCEPT
{
    SkrZoneScopedN("THREAD_SERVICE_BODY");
    for (;;)
    {
        auto S = _service->takeAction();
        if (S == kStatusRunning)
        {
            // running...
            SkrZoneScopedN("RUNNING");
            {
                // 1. run service
                auto R = _service->serve();
                // 2. check result
                if (R != ASYNC_RESULT_OK)
                    return R;
            }
        }
        else if (S == kStatusStopped)
        {
            skr_thread_sleep(0);
        }
        else if (S == kStatusExitted)
        {
            SkrZoneScopedN("EXIT");
            return ASYNC_RESULT_OK;
        }
    }
    return ASYNC_RESULT_OK;
}

void AsyncService::sleep() SKR_NOEXCEPT
{
    const auto ms = skr_atomicu32_load_relaxed(&sleep_time);
    SkrZoneScopedNC("asyncService(Cond)", tracy::Color::Gray55);

    condlock.lock();
    if (!event)
    {
        condlock.wait(ms);
    }
    event = false;
    condlock.unlock();
}

} // namespace skr