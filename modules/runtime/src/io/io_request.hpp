#pragma once
#include "pool.hpp"

#include <string.h> // ::strlen
#include "tracy/Tracy.hpp"

namespace skr {
namespace io {

typedef enum SkrAsyncIODoneStatus
{
    SKR_ASYNC_IO_DONE_STATUS_NEED = 1,
    SKR_ASYNC_IO_DONE_STATUS_PENDING = 2,
    SKR_ASYNC_IO_DONE_STATUS_DONE = 3
} SkrAsyncIODoneStatus;

constexpr const char* callback_names[SKR_IO_STAGE_COUNT] = {
    "IOCallback(None)",
    "IOCallback(Enqueued)",
    "IOCallback(Sorting)",
    "IOCallback(Resolving)",
    "IOCallback(Loading)",
    "IOCallback(Loaded)",
    "IOCallback(Decompressing)",
    "IOCallback(Completed)",
    "IOCallback(Cancelled)",
};

struct IORequestBase : public IIORequest
{
    void setStatus(ESkrIOStage status)
    {
        skr_atomicu32_store_release(&future->status, status);
        if (const auto callback = callbacks[status])
        {
            ZoneScoped;
            ZoneName(callback_names[status], ::strlen(callback_names[status]));

            callback(future, nullptr, callback_datas[status]);
        }
    }

    ESkrIOStage getStatus() const
    {
        return static_cast<ESkrIOStage>(skr_atomicu32_load_relaxed(&future->status));
    }
    
    float sub_priority;
    SAtomic32 done = 0;
    skr_io_future_t* future = nullptr;

    skr_io_callback_t callbacks[SKR_IO_STAGE_COUNT];
    void* callback_datas[SKR_IO_STAGE_COUNT];

    skr_io_callback_t finish_callbacks[SKR_IO_FINISH_POINT_COUNT];
    void* finish_callback_datas[SKR_IO_FINISH_POINT_COUNT];
};

} // namespace io
} // namespace skr