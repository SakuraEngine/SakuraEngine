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
public:
    void tryPollFinish() SKR_NOEXCEPT
    {
        if (getStatus() == SKR_IO_STAGE_COMPLETED)
        {
            finish_callbacks[SKR_IO_FINISH_POINT_COMPLETE](
                future, this, finish_callback_datas[SKR_IO_FINISH_POINT_COMPLETE]);
        }
        else
        {
            finish_callbacks[SKR_IO_FINISH_POINT_CANCEL](
                future, this, finish_callback_datas[SKR_IO_FINISH_POINT_CANCEL]);
        }
        skr_atomicu32_store_relaxed(&done, SKR_ASYNC_IO_DONE_STATUS_DONE);
    }

    bool needPollFinish() SKR_NOEXCEPT
    {            
        for (auto f : finish_callbacks)
            return f;
        return false;
    }

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
        
    void set_sub_priority(float sub_pri) SKR_NOEXCEPT { sub_priority = sub_pri; }
    float get_sub_priority() const SKR_NOEXCEPT { return sub_priority; }

    float sub_priority;
    SAtomic32 done = 0;
    skr_io_future_t* future = nullptr;

public:
    SInterfaceDeleter custom_deleter() const 
    { 
        return +[](SInterface* ptr) 
        { 
            auto* p = static_cast<IORequestBase*>(ptr);
            p->pool->deallocate(p); 
        };
    }
    IORequestBase(ISmartPool<IIORequest>* pool) : pool(pool) {}
protected:
    ISmartPool<IIORequest>* pool = nullptr;

protected:
    skr_io_callback_t callbacks[SKR_IO_STAGE_COUNT];
    void* callback_datas[SKR_IO_STAGE_COUNT];

    skr_io_callback_t finish_callbacks[SKR_IO_FINISH_POINT_COUNT];
    void* finish_callback_datas[SKR_IO_FINISH_POINT_COUNT];
    
public:
    uint32_t add_refcount() 
    { 
        return 1 + skr_atomicu32_add_relaxed(&rc, 1); 
    }
    uint32_t release() 
    {
        skr_atomicu32_add_relaxed(&rc, -1);
        return skr_atomicu32_load_acquire(&rc);
    }
private:
    SAtomicU32 rc = 0;

};

} // namespace io
} // namespace skr