#include "SkrRT/async/async_service.h"
#include "SkrRT/async/wait_timeout.hpp"
#include "SkrRT/misc/log.hpp"
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

void ServiceThread::request_stop() SKR_NOEXCEPT
{
    setAction(kActionStop);
}

void ServiceThread::stop() SKR_NOEXCEPT
{
    SkrZoneScopedN("stop");
    request_stop();
    wait_stop();
}

void ServiceThread::wait_stop(uint32_t fatal_timeout) SKR_NOEXCEPT
{
    SkrZoneScopedN("wait_stop");
    waitStatus(kStatusStopped, fatal_timeout);
}

void ServiceThread::request_run() SKR_NOEXCEPT
{
    if (!t.has_started())
    {
        t.start(&f);
    }
    setAction(kActionWake);
}

void ServiceThread::run() SKR_NOEXCEPT
{
    SkrZoneScopedN("run");
    request_run();
    wait_run();
}

void ServiceThread::wait_run(uint32_t fatal_timeout) SKR_NOEXCEPT
{
    SkrZoneScopedN("wait_run");
    waitStatus(kStatusRunning, fatal_timeout);
}

void ServiceThread::request_exit() SKR_NOEXCEPT
{
    setAction(kActionExit);
}

void ServiceThread::exit() SKR_NOEXCEPT
{
    SkrZoneScopedN("exit");
    request_exit();
    wait_exit();

    t.finalize();
}

void ServiceThread::wait_exit(uint32_t fatal_timeout) SKR_NOEXCEPT
{
    SkrZoneScopedN("wait_exit");
    waitStatus(kStatusExitted, fatal_timeout);
}

void ServiceThread::setAction(Action target) SKR_NOEXCEPT
{
    const auto act = get_action();

    if ((target != kActionNone) && (act != kActionNone) && (target != act))
        SKR_LOG_FATAL(u8"service_thread: request_xxx is handling,"
                      " current action: %d, to request: %d",
                      act, target);

    skr_atomic32_store_release(&action_, target);
    SKR_LOG_FMT_BACKTRACE(u8"service_thread: action set to: {}.", (int32_t)target);
}

void ServiceThread::setStatus(Status target) SKR_NOEXCEPT
{
    const auto S = get_status();
    if (target == kStatusStopped)
    {
        if (S == kStatusExitted)
        {
            SKR_LOG_FATAL(u8"service_thread: must stop from a running/stopped service! current status: %d", S);
            SKR_ASSERT(S != kStatusExitted);
        }
    }
    else if (target == kStatusRunning)
    {
        if (S != kStatusStopped)
        {
            SKR_LOG_FATAL(u8"service_thread: must wake from a stopped service! current status: %d", S);
            SKR_ASSERT(S == kStatusStopped);
        }
    }
    else if (target == kStatusExitted)
    {
        if (S != kStatusStopped)
        {
            SKR_LOG_FATAL(u8"service_thread: must exit from a stopped service! current status: %d", S);
            SKR_ASSERT(S == kStatusStopped);
        }
    }
    skr_atomic32_store_release(&status_, target);
    SKR_LOG_FMT_BACKTRACE(u8"service_thread: status set to: {}.", (int32_t)target);
}

void ServiceThread::waitStatus(Status target, uint32_t fatal_timeout) SKR_NOEXCEPT
{
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
    wait_timeout([&] { return get_status() == target; }, fatal_timeout);
}

void ServiceThread::waitJoin() SKR_NOEXCEPT
{
    wait_exit();
    t.join();
}

ServiceThread::Status ServiceThread::takeAction() SKR_NOEXCEPT
{
    const auto act = get_action();
    setAction(kActionNone);

    const Status lut[] = { kStatusRunning, kStatusStopped, kStatusExitted };
    if (act != kActionNone)
    {
        const auto out = lut[act - 1];
        setStatus(out);
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