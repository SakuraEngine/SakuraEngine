#pragma once
#include "SkrRT/config.h"
#ifdef __cplusplus
    #include "SkrRT/async/named_thread.hpp"
    #include "SkrRT/async/condlock.hpp"
#endif

typedef enum SkrAsyncServiceStatus
{
    SKR_ASYNC_SERVICE_STATUS_SLEEPING = 0,
    SKR_ASYNC_SERVICE_STATUS_RUNNING  = 1,
    SKR_ASYNC_SERVICE_STATUS_QUITING  = 2,
    SKR_ASYNC_SERVICE_STATUS_COUNT,
    SKR_ASYNC_SERVICE_STATUS_MAX_ENUM = UINT32_MAX
} SkrAsyncServiceStatus;

typedef enum SkrAsyncServicePriority
{
    SKR_ASYNC_SERVICE_PRIORITY_URGENT   = 0,
    SKR_ASYNC_SERVICE_PRIORITY_NORMAL   = 1,
    SKR_ASYNC_SERVICE_PRIORITY_LOW      = 2,
    SKR_ASYNC_SERVICE_PRIORITY_COUNT    = 3,
    SKR_ASYNC_SERVICE_PRIORITY_MAX_ENUM = INT32_MAX
} SkrAsyncServicePriority;

#ifdef __cplusplus

namespace skr
{
struct ServiceThreadDesc {
    const char8_t*  name     = nullptr;
    SThreadPriority priority = SKR_THREAD_NORMAL;
};

struct SKR_STATIC_API ServiceThread {
public:
    ServiceThread(const ServiceThreadDesc& desc) SKR_NOEXCEPT;
    virtual ~ServiceThread() SKR_NOEXCEPT;

    enum Status
    {
        kStatusStopped  = 0,
        kStatusRunning  = 1,
        kStatusExitted  = 5
    };
    enum Action
    {
        kActionNone = 0,
        kActionWake = 1,
        kActionStop = 2,
        kActionExit = 3
    };
    Status get_status() const SKR_NOEXCEPT;
    Action get_action() const SKR_NOEXCEPT;

    virtual void request_stop() SKR_NOEXCEPT;
    void stop() SKR_NOEXCEPT;
    virtual void wait_stop(uint32_t fatal_timeout = 8) SKR_NOEXCEPT;

    void request_run() SKR_NOEXCEPT;
    void run() SKR_NOEXCEPT;
    void wait_run(uint32_t fatal_timeout = 8) SKR_NOEXCEPT;

    void request_exit() SKR_NOEXCEPT;
    void exit() SKR_NOEXCEPT;
    void wait_exit(uint32_t fatal_timeout = 8) SKR_NOEXCEPT;

    virtual AsyncResult serve() SKR_NOEXCEPT = 0;

protected:
    Status takeAction() SKR_NOEXCEPT;
    void setStatus(Status status) SKR_NOEXCEPT;
    void setAction(Action action) SKR_NOEXCEPT;

    void waitStatus(Status status, uint32_t fatal_timeout) SKR_NOEXCEPT;
    void waitJoin() SKR_NOEXCEPT;

    struct ServiceFunc : public NamedThreadFunction {
        AsyncResult    run() SKR_NOEXCEPT;
        ServiceThread* _service = nullptr;
    };
    friend struct ServiceFunc;
    ServiceFunc f;
    NamedThread t;
private:
    SAtomic32 action = kActionNone;
    SAtomic32 status = kStatusStopped;
};

struct SKR_STATIC_API AsyncService : public skr::ServiceThread {
    AsyncService(const ServiceThreadDesc& desc) SKR_NOEXCEPT
        : skr::ServiceThread(desc)
    {
        condlock.initialize(skr::format(u8"{}-CondLock", desc.name).u8_str());
    }
    virtual ~AsyncService() SKR_NOEXCEPT = default;

    SkrAsyncServiceStatus getServiceStatus() const SKR_NOEXCEPT
    {
        return (SkrAsyncServiceStatus)skr_atomicu32_load_acquire(&service_status);
    }

    void set_sleep_time(uint32_t time) SKR_NOEXCEPT
    {
        skr_atomicu32_store_release(&sleep_time, time);
    }

    void sleep() SKR_NOEXCEPT;

    void request_stop() SKR_NOEXCEPT override
    {
        skr::ServiceThread::request_stop();
        awake();
    }

    void wait_stop(uint32_t fatal_timeout = 8) SKR_NOEXCEPT override
    {
        awake();
        skr::ServiceThread::wait_stop(fatal_timeout);
    }

    void awake() SKR_NOEXCEPT
    {
        condlock.lock();
        event = true;
        condlock.signal();
        condlock.unlock();
    }

protected:
    void setServiceStatus(SkrAsyncServiceStatus status) SKR_NOEXCEPT
    {
        skr_atomicu32_store_release(&service_status, status);
    }

private:
    SAtomicU32 sleep_time = 16u;
    bool       event      = false;
    CondLock   condlock;
    SAtomicU32 service_status = SKR_ASYNC_SERVICE_STATUS_SLEEPING;
};

} // namespace skr
#endif