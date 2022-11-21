#pragma once
#include "io.h"

typedef struct skr_service_task_t {
    SkrAsyncServicePriority priority SKR_IF_CPP(= SKR_ASYNC_SERVICE_PRIORITY_NORMAL);
    float sub_priority SKR_IF_CPP(= 0.f); /*0.f ~ 1.f*/
    skr_async_callback_t callbacks[SKR_ASYNC_IO_STATUS_COUNT];
    void* callback_datas[SKR_ASYNC_IO_STATUS_COUNT];
} skr_service_task_t;

typedef struct skr_threaded_service_desc_t {
    const char8_t* name SKR_IF_CPP(= nullptr);
    uint32_t sleep_time SKR_IF_CPP(= SKR_ASYNC_SERVICE_SLEEP_TIME_MAX);
    bool lockless SKR_IF_CPP(= true);
    SkrAsyncServiceSortMethod sort_method SKR_IF_CPP(= SKR_ASYNC_SERVICE_SORT_METHOD_NEVER);
    SkrAsyncServiceSleepMode sleep_mode SKR_IF_CPP(= SKR_ASYNC_SERVICE_SLEEP_MODE_COND_VAR);
} skr_threaded_service_desc_t;

#ifdef __cplusplus
struct RUNTIME_API skr_threaded_service_t
{
public:
    [[nodiscard]] static skr_threaded_service_t* create(const skr_threaded_service_desc_t* desc) SKR_NOEXCEPT;
    static void destroy(skr_threaded_service_t* service) SKR_NOEXCEPT;

    // enqueue a request
    virtual void request(const skr_service_task_t* task, skr_async_request_t* async_request) SKR_NOEXCEPT = 0;

    // try to cancel an enqueued request at **this** thread
    // not available (returns always false) under lockless mode
    // returns false if the request is under LOADING status
    virtual bool try_cancel(skr_async_request_t* request) SKR_NOEXCEPT = 0;

    // emplace a cancel **command** to ioService thread
    // it's recommended to use this under lockless mode
    virtual void defer_cancel(skr_async_request_t* request) SKR_NOEXCEPT = 0;

    // stop service and hang up underground thread
    virtual void stop(bool wait_drain = false) SKR_NOEXCEPT = 0;

    // start & run service
    virtual void run() SKR_NOEXCEPT = 0;

    // block & finish up all requests
    virtual void drain() SKR_NOEXCEPT = 0;

    // set sleep time when io queue is detected to be idle
    virtual void set_sleep_time(uint32_t time) SKR_NOEXCEPT = 0;

    // get service status (sleeping or running)
    virtual SkrAsyncServiceStatus get_service_status() const SKR_NOEXCEPT = 0;

    virtual ~skr_threaded_service_t() SKR_NOEXCEPT = default;
    skr_threaded_service_t() SKR_NOEXCEPT = default;
};
#else
typedef struct skr_threaded_service_t skr_threaded_service_t;
#endif