#pragma once
#include "../common/io_request.hpp"
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
    friend struct SmartPool<RAMIORequest, IIORequest>;

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

    skr::span<skr_io_block_t> get_blocks() SKR_NOEXCEPT { return blocks; }
    void add_block(const skr_io_block_t& block) SKR_NOEXCEPT { blocks.emplace_back(block); }
    void reset_blocks() SKR_NOEXCEPT { blocks.clear(); }

    skr::span<skr_io_compressed_block_t> get_compressed_blocks() SKR_NOEXCEPT { return {}; }
    void add_compressed_block(const skr_io_block_t& block) SKR_NOEXCEPT {  }
    void reset_compressed_blocks() SKR_NOEXCEPT {  }

protected:
    RAMIORequest(ISmartPool<IIORequest>* pool, const uint64_t sequence) : IORequestBase(pool), sequence(sequence) {}

    const uint64_t sequence;
};

using RQPtr = skr::SObjectPtr<RAMIORequest>;
using IORequestQueue = moodycamel::ConcurrentQueue<RQPtr>;  
using IORequestArray = skr::vector<RQPtr>;

} // namespace io
} // namespace skr