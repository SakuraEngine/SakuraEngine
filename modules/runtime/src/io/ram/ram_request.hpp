#pragma once
#include "../common/io_request.hpp"
#include "ram_buffer.hpp"
#include "platform/vfs.h"
#include <EASTL/fixed_vector.h>
#include <EASTL/variant.h>

#include <string.h> // ::strlen

namespace skr {
namespace io {

struct RAMIORequest final : public IORequestBase
{
    friend struct SmartPool<RAMIORequest, IIORequest>;

    RAMIOBufferId destination = nullptr;
    eastl::fixed_vector<skr_io_block_t, 1> blocks;
    
    uint64_t get_fsize() const SKR_NOEXCEPT override
    {
        if (file)
        {
            SKR_ASSERT(!dfile);
            return skr_vfs_fsize(file);
        }
        else
        {
            SKR_ASSERT(dfile);
            SKR_ASSERT(!file);
            auto instance = skr_get_dstorage_instnace();
            SkrDStorageFileInfo info;
            skr_dstorage_query_file_info(instance, dfile, &info);
            return info.file_size;
        }
    }

    void setStatus(ESkrIOStage status) SKR_NOEXCEPT override
    {
        if (status == SKR_IO_STAGE_CANCELLED)
        {
            if (auto dest = static_cast<RAMIOBuffer*>(destination.get()))
            {
                dest->free_buffer();
            }
        }
        return IORequestBase::setStatus(status);
    }

    skr::span<skr_io_block_t> get_blocks() SKR_NOEXCEPT override { return blocks; }
    void add_block(const skr_io_block_t& block) SKR_NOEXCEPT override { blocks.emplace_back(block); }
    void reset_blocks() SKR_NOEXCEPT override { blocks.clear(); }

    skr::span<skr_io_compressed_block_t> get_compressed_blocks() SKR_NOEXCEPT override { return {}; }
    void add_compressed_block(const skr_io_block_t& block) SKR_NOEXCEPT override {  }
    void reset_compressed_blocks() SKR_NOEXCEPT override {  }

protected:
    RAMIORequest(ISmartPool<IIORequest>* pool, const uint64_t sequence) : IORequestBase(pool), sequence(sequence) {}

    const uint64_t sequence;
};

using RAMRQPtr = skr::SObjectPtr<IORequestBase>;


} // namespace io
} // namespace skr