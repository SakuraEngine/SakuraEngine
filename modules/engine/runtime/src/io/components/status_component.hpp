#pragma once
#include "SkrRT/io/io.h"
#include "../components/component.hpp"

#include "SkrProfile/profile.h"

namespace skr {
namespace io {

constexpr const char* callback_names[] = {
    "IOCallback(None)",
    "IOCallback(Enqueued)",
    "IOCallback(Resolving)",
    "IOCallback(Loading)",
    "IOCallback(Loaded)",
    "IOCallback(Decompressing)",
    "IOCallback(Decompressed)",
    "IOCallback(Completed)",
    "IOCallback(Cancelled)",
};
static_assert(sizeof(callback_names) / sizeof(callback_names[0]) == SKR_IO_STAGE_COUNT, "callback_names size mismatch");

typedef enum SkrAsyncIOFinishStep
{
    SKR_ASYNC_IO_FINISH_STEP_NONE = 0,
    SKR_ASYNC_IO_FINISH_STEP_WAIT_CALLBACK_POLLING = 1,
    SKR_ASYNC_IO_FINISH_STEP_DONE = 2
} SkrAsyncIOFinishStep;

template <>
struct CID<struct IOStatusComponent> 
{
    static constexpr skr_guid_t Get();
};
struct SKR_RUNTIME_API IOStatusComponent : public IORequestComponent
{
public:
    IOStatusComponent(IIORequest* const request) SKR_NOEXCEPT;
    virtual ~IOStatusComponent() SKR_NOEXCEPT;

    const skr_io_future_t* get_future() const SKR_NOEXCEPT { return future; }

    void add_callback(ESkrIOStage stage, skr_io_callback_t callback, void* data) SKR_NOEXCEPT 
    { 
        callbacks[stage] = callback; 
        callback_datas[stage] = data; 
    }

    void add_finish_callback(ESkrIOFinishPoint point, skr_io_callback_t callback, void* data) SKR_NOEXCEPT
    {
        finish_callbacks[point] = callback;
        finish_callback_datas[point] = data;
    }

    virtual ESkrIOStage getStatus() const SKR_NOEXCEPT
    {
        return static_cast<ESkrIOStage>(skr_atomic_load_relaxed(&future->status));
    }

    bool getCancelRequested() const SKR_NOEXCEPT
    {
        return skr_atomic_load_relaxed(&future->request_cancel);
    }

    SkrAsyncIOFinishStep getFinishStep() const SKR_NOEXCEPT
    { 
        return (SkrAsyncIOFinishStep)skr_atomic_load_acquire(&finish_step); 
    }

    void setFinishStep(SkrAsyncIOFinishStep step) SKR_NOEXCEPT
    { 
        skr_atomic_store_release(&finish_step, step); 
    }

    void tryPollFinish() SKR_NOEXCEPT
    {
        if (getStatus() == SKR_IO_STAGE_COMPLETED)
        {
            finish_callbacks[SKR_IO_FINISH_POINT_COMPLETE](
                future, request, finish_callback_datas[SKR_IO_FINISH_POINT_COMPLETE]);
        }
        else
        {
            finish_callbacks[SKR_IO_FINISH_POINT_CANCEL](
                future, request, finish_callback_datas[SKR_IO_FINISH_POINT_CANCEL]);
        }
        skr_atomic_store_relaxed(&finish_step, SKR_ASYNC_IO_FINISH_STEP_DONE);
    }

    bool needPollFinish() SKR_NOEXCEPT
    {            
        for (auto f : finish_callbacks)
            return f;
        return false;
    }

    virtual void setStatus(ESkrIOStage status) SKR_NOEXCEPT
    {
        skr_atomic_store_release(&future->status, status);
        if (const auto callback = callbacks[status])
        {
            SkrZoneScoped;
            ZoneName(callback_names[status], ::strlen(callback_names[status]));
            callback(future, request, callback_datas[status]);
        }
    }

    IIOBatch* getOwnerBatch() const SKR_NOEXCEPT { return owner_batch; }
    void use_async_complete() SKR_NOEXCEPT { async_complete = true; }
    void use_async_cancel() SKR_NOEXCEPT { async_cancel = true; }
    bool is_async_complete() const SKR_NOEXCEPT { return async_complete; }
    bool is_async_cancel() const SKR_NOEXCEPT { return async_cancel; }

protected:
    friend struct RAMIOBatch;
    friend struct VRAMIOBatch;
    bool async_complete = false;
    bool async_cancel = false;
    IIOBatch* owner_batch = nullptr; // avoid circular reference

    skr_io_future_t* future = nullptr;
    SAtomic32 finish_step = 0;
    skr_io_callback_t callbacks[SKR_IO_STAGE_COUNT] = { nullptr };
    void* callback_datas[SKR_IO_STAGE_COUNT];

    skr_io_callback_t finish_callbacks[SKR_IO_FINISH_POINT_COUNT] = { nullptr };
    void* finish_callback_datas[SKR_IO_FINISH_POINT_COUNT];
};

constexpr skr_guid_t CID<struct IOStatusComponent>::Get()
{
    using namespace skr::literals;
    return u8"3db75617-8027-464b-b241-e4e59f83fd61"_guid;
} 

} // namespace io
} // namespace skr