#pragma once
#include "../common/io_request.hpp"
#include "SkrRT/platform/vfs.h"

#include <EASTL/fixed_vector.h>
#include <EASTL/variant.h>
#include <string.h> // ::strlen

namespace skr {
namespace io {

struct VRAMIORequest final : public IORequestCRTP<IIORequest, IOFileComponent, IOStatusComponent>
{
    friend struct SmartPool<VRAMIORequest, IIORequest>;

    eastl::fixed_vector<skr_io_block_t, 1> blocks;
    
    uint64_t get_fsize() const SKR_NOEXCEPT
    {
        if (auto pFile = get_component<IOFileComponent>(this))
        {
            if (pFile->file)
            {
                SKR_ASSERT(!pFile->dfile);
                return skr_vfs_fsize(pFile->file);
            }
            else
            {
                SKR_ASSERT(pFile->dfile);
                SKR_ASSERT(!pFile->file);
                auto instance = skr_get_dstorage_instnace();
                SkrDStorageFileInfo info;
                skr_dstorage_query_file_info(instance, pFile->dfile, &info);
                return info.file_size;
            }
        }
        return 0;
    }

    void setStatus(ESkrIOStage status) SKR_NOEXCEPT
    {
        if (auto pStatus = get_component<IOStatusComponent>(this))
        {
            /*
            if (status == SKR_IO_STAGE_CANCELLED)
            {
                if (auto dest = static_cast<RAMIOBuffer*>(destination.get()))
                {
                    dest->free_buffer();
                }
            }
            */
            return pStatus->setStatus(status);
        }
    }

    skr::span<skr_io_block_t> get_blocks() SKR_NOEXCEPT override { return blocks; }
    void add_block(const skr_io_block_t& block) SKR_NOEXCEPT override { blocks.emplace_back(block); }
    void reset_blocks() SKR_NOEXCEPT override { blocks.clear(); }

    skr::span<skr_io_compressed_block_t> get_compressed_blocks() SKR_NOEXCEPT override { return {}; }
    void add_compressed_block(const skr_io_block_t& block) SKR_NOEXCEPT override {  }
    void reset_compressed_blocks() SKR_NOEXCEPT override {  }

protected:
    VRAMIORequest(ISmartPool<IIORequest>* pool, const uint64_t sequence) 
        : IORequestCRTP(pool), sequence(sequence) 
    {

    }

    const uint64_t sequence;
};

using VRAMRQPtr = skr::SObjectPtr<VRAMIORequest>;

} // namespace io
} // namespace skr