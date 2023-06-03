#pragma once
#include "../io_request.hpp"
#include "platform/vfs.h"
#include "misc/log.h"
#include "misc/defer.hpp"
#include "async/condlock.hpp"
#include "async/service_thread.hpp"
#include "containers/hashmap.hpp"
#include "containers/sptr.hpp"
#include <EASTL/fixed_vector.h>
#include <EASTL/variant.h>

#include <string.h> // ::strlen
#include "tracy/Tracy.hpp"

namespace skr {
namespace io {

struct RAMIORequest final : public IORequestBase
{
    skr_vfs_t* vfs = nullptr;
    skr::string path;
    skr_io_file_handle file;

    RAMIOBufferId destination = nullptr;

    eastl::fixed_vector<skr_io_block_t, 1> blocks;

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

public:
    SInterfaceDeleter custom_deleter() const 
    { 
        return +[](SInterface* ptr) 
        { 
            auto* p = static_cast<RAMIORequest*>(ptr);
            p->pool->deallocate(p); 
        };
    }
    friend struct SmartPool<RAMIORequest, IIORequest>;
protected:
    RAMIORequest(ISmartPool<IIORequest>* pool, const uint64_t sequence) : sequence(sequence), pool(pool) {}

    const uint64_t sequence;
    ISmartPool<IIORequest>* pool = nullptr;
};

using RQPtr = skr::SObjectPtr<RAMIORequest>;
using IORequestQueue = moodycamel::ConcurrentQueue<RQPtr>;  
using IORequestArray = skr::vector<RQPtr>;

} // namespace io
} // namespace skr