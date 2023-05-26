#include "async/service_thread.hpp"

namespace skr
{
ServiceThread::ServiceThread(const ServiceThreadDesc* desc) SKR_NOEXCEPT
{
    f._service = this;

    NamedThreadDesc tDesc = {};
    tDesc.name = desc->name;
    tDesc.priority = SKR_THREAD_NORMAL;
    tDesc.stack_size = 16 * 1024;
    t.initialize(&tDesc);
}

ServiceThread::Status ServiceThread::get_status() const SKR_NOEXCEPT
{
    return static_cast<Status>(skr_atomic32_load_acquire(&status));
}

void ServiceThread::stop() SKR_NOEXCEPT
{
    skr_atomic32_store_release(&status, kStatusStopping);
}

void ServiceThread::run() SKR_NOEXCEPT
{
    t.start(&f);
}

AsyncResult ServiceThread::ServiceFunc::run() SKR_NOEXCEPT
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
        else if (S == kStatusNone)
        {
            SKR_ASSERT(0 && "ServiceThread::serve() must set status to kStatusRunning or kStatusStopping");
        }
        else if (S == kStatusStopping)
        {
            goto STOP;
        }
        else if (S == kStatusStopped)
        {
            SKR_ASSERT(0 && "ServiceThread::serve() must set status to kStatusRunning or kStatusStopping");
        }
    }
    return ASYNC_RESULT_OK;
STOP:
    skr_atomic32_store_release(&_service->status, kStatusStopped);
    return ASYNC_RESULT_OK;
}

}