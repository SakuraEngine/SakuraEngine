#pragma once
#include "io/io.h"
#include "platform/vfs.h"
#include "misc/log.h"
#include "misc/defer.hpp"
#include "async/condlock.hpp"
#include "async/service_thread.hpp"
#include "containers/hashmap.hpp"
#include "containers/sptr.hpp"
#include <EASTL/fixed_vector.h>
#include <EASTL/variant.h>

#include "tracy/Tracy.hpp"

namespace skr {
namespace io {

typedef enum SkrAsyncIODoneStatus
{
    SKR_ASYNC_IO_DONE_STATUS_PENDING = 1,
    SKR_ASYNC_IO_DONE_STATUS_DONE = 2
} SkrAsyncIODoneStatus;

struct RAMIORequest final : public IIORequest
{
    RAMIORequest(const uint64_t sequence) : sequence(sequence) {}

    const uint64_t sequence;
    skr_vfs_t* vfs = nullptr;
    skr::string path;
    skr_io_file_handle file;
    
    SkrAsyncServicePriority priority;
    float sub_priority;

    SAtomic32 done = 0;
    skr_io_future_t* future = nullptr;
    skr_async_ram_destination_t* destination = nullptr;

    skr_io_callback_t callbacks[SKR_IO_STAGE_COUNT];
    void* callback_datas[SKR_IO_STAGE_COUNT];

    skr_io_callback_t finish_callbacks[SKR_IO_FINISH_POINT_COUNT];
    void* finish_callback_datas[SKR_IO_FINISH_POINT_COUNT];

    eastl::fixed_vector<skr_io_block_t, 1> blocks;

    void setStatus(ESkrIOStage status)
    {
        skr_atomicu32_store_release(&future->status, status);
        if (const auto callback = callbacks[status])
        {
            callback(future, nullptr, callback_datas[status]);
        }
    }

    ESkrIOStage getStatus() const
    {
        return static_cast<ESkrIOStage>(skr_atomicu32_load_relaxed(&future->status));
    }

    void set_vfs(skr_vfs_t* _vfs) SKR_NOEXCEPT { vfs = _vfs; }
    void set_path(const char8_t* p) SKR_NOEXCEPT { path = p; }
    virtual const char8_t* get_path() const SKR_NOEXCEPT { return path.u8_str(); }
    
    void open_file() SKR_NOEXCEPT
    {
        SKR_ASSERT(vfs);
        if (!file)
        {
            file = skr_vfs_fopen(vfs, path.u8_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
        }
    }

    uint64_t get_fsize() const SKR_NOEXCEPT
    {
        SKR_ASSERT(file);
        return skr_vfs_fsize(file);
    }
    
    void set_priority(SkrAsyncServicePriority pri) SKR_NOEXCEPT { priority = pri; }
    SkrAsyncServicePriority get_priority() const SKR_NOEXCEPT { return priority; }

    void set_sub_priority(float sub_pri) SKR_NOEXCEPT { sub_priority = sub_pri; }
    float get_sub_priority() const SKR_NOEXCEPT { return sub_priority; }

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

    skr::span<skr_io_block_t> get_blocks() SKR_NOEXCEPT { return blocks; }
    void add_block(const skr_io_block_t& block) SKR_NOEXCEPT { blocks.emplace_back(block); }
    void reset_blocks() SKR_NOEXCEPT { blocks.clear(); }

    skr::span<skr_io_compressed_block_t> get_compressed_blocks() SKR_NOEXCEPT { return {}; }
    void add_compressed_block(const skr_io_block_t& block) SKR_NOEXCEPT {  }
    void reset_compressed_blocks() SKR_NOEXCEPT {  }

    uint32_t add_refcount() 
    { 
        return 1 + skr_atomicu32_add_relaxed(&rc, 1); 
    }

    uint32_t release() 
    {
        skr_atomicu32_add_relaxed(&rc, -1);
        return skr_atomicu32_load_acquire(&rc);
    }

    SInterfaceDeleter custom_deleter() const 
    { 
        return nullptr;
    }

private:
    SAtomicU32 rc = 0;
};

using RQPtr = skr::SObjectPtr<RAMIORequest>;
using IORequestQueue = moodycamel::ConcurrentQueue<RQPtr>;  
using IORequestArray = skr::vector<RQPtr>;

} // namespace io
} // namespace skr