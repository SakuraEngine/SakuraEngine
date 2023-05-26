#pragma once
#include "async/named_thread.hpp"

namespace skr
{
struct ServiceThreadDesc
{
    const char8_t* name = nullptr;
};

struct RUNTIME_STATIC_API ServiceThread
{
public:
    ServiceThread(const ServiceThreadDesc& desc) SKR_NOEXCEPT;
    virtual ~ServiceThread() SKR_NOEXCEPT;
    
    enum Status
    {
        kStatusRunning = 0,
        kStatusWaking = 1,
        kStatusStopping = 2,
        kStatusStopped = 3,
        kStatusExiting = 4,
        kStatusExitted = 5
    };
    Status get_status() const SKR_NOEXCEPT;
    
    void request_stop() SKR_NOEXCEPT;
    void stop() SKR_NOEXCEPT;
    void wait_stop() SKR_NOEXCEPT;

    void request_run() SKR_NOEXCEPT;
    void run() SKR_NOEXCEPT;
    void wait_running() SKR_NOEXCEPT;

    void request_exit() SKR_NOEXCEPT;
    void exit() SKR_NOEXCEPT;
    void wait_exit() SKR_NOEXCEPT;

    virtual AsyncResult serve() SKR_NOEXCEPT = 0;

protected:
    void waitJoin() SKR_NOEXCEPT;

    struct ServiceFunc : public NamedThreadFunction
    {
        AsyncResult run() SKR_NOEXCEPT;
        ServiceThread* _service = nullptr;
    };
    friend struct ServiceFunc;
    ServiceFunc f;
    NamedThread t;
    SAtomicU32 status = kStatusStopped;
};
}