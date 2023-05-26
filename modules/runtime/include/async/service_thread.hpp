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
    ServiceThread(const ServiceThreadDesc* desc) SKR_NOEXCEPT;

    enum Status
    {
        kStatusNone = 0,
        kStatusRunning = 1,
        kStatusStopping = 2,
        kStatusStopped = 3
    };

    virtual AsyncResult serve() SKR_NOEXCEPT = 0;

    Status get_status() const SKR_NOEXCEPT;
    void stop() SKR_NOEXCEPT;
    void run() SKR_NOEXCEPT;
protected:
    struct ServiceFunc : public NamedThreadFunction
    {
        AsyncResult run() SKR_NOEXCEPT;
        ServiceThread* _service = nullptr;
    };
    friend struct ServiceFunc;
    ServiceFunc f;
    NamedThread t;
    SAtomicU32 status;
};
}