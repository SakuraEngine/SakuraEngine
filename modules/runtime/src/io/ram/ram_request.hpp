#pragma once
#include "../common/io_request.hpp"
#include "../components/blocks_component.hpp"
#include "ram_buffer.hpp"
#include "SkrRT/platform/vfs.h"

#include <EASTL/fixed_vector.h>
#include <EASTL/variant.h>
#include <string.h> // ::strlen

namespace skr {
namespace io {

struct RAMIORequest final : public IORequestCRTP<IIORequest, 
    IOFileComponent, IOStatusComponent, IOBlocksComponent>
{
    friend struct SmartPool<RAMIORequest, IIORequest>;

    RAMIOBufferId destination = nullptr;
    
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
            if (status == SKR_IO_STAGE_CANCELLED)
            {
                if (auto dest = static_cast<RAMIOBuffer*>(destination.get()))
                {
                    dest->free_buffer();
                }
            }
            return pStatus->setStatus(status);
        }
    }

    skr::span<skr_io_block_t> get_blocks() SKR_NOEXCEPT override 
    { 
        return get_component<IOBlocksComponent>(this)->get_blocks(); 
    }
    void add_block(const skr_io_block_t& block) SKR_NOEXCEPT override 
    { 
        get_component<IOBlocksComponent>(this)->add_block(block); 
    }
    void reset_blocks() SKR_NOEXCEPT override 
    { 
        get_component<IOBlocksComponent>(this)->reset_blocks(); 
    }
    
    skr::span<skr_io_compressed_block_t> get_compressed_blocks() SKR_NOEXCEPT override { return {}; }
    void add_compressed_block(const skr_io_block_t& block) SKR_NOEXCEPT override {  }
    void reset_compressed_blocks() SKR_NOEXCEPT override {  }

protected:
    RAMIORequest(ISmartPool<IIORequest>* pool, const uint64_t sequence) 
        : IORequestCRTP(pool), sequence(sequence) 
    {

    }

    const uint64_t sequence;
};

using RAMRQPtr = skr::SObjectPtr<RAMIORequest>;

} // namespace io
} // namespace skr