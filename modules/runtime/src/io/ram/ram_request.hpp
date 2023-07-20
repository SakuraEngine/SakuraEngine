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

struct RAMIOStatusComponent final : public IOStatusComponent
{
    RAMIOStatusComponent(IIORequest* const request) SKR_NOEXCEPT 
        : IOStatusComponent(request) 
    {
        
    }
    void setStatus(ESkrIOStage status) SKR_NOEXCEPT override;
};

struct RAMIORequest final : public IORequestCRTP<IIORequest, 
    IOFileComponent, RAMIOStatusComponent, IOBlocksComponent>
{
    friend struct SmartPool<RAMIORequest, IIORequest>;

    RAMIOBufferId destination = nullptr;
    
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

inline void RAMIOStatusComponent::setStatus(ESkrIOStage status) SKR_NOEXCEPT
{
    auto rq = static_cast<RAMIORequest*>(request);
    if (status == SKR_IO_STAGE_CANCELLED)
    {
        if (auto dest = static_cast<RAMIOBuffer*>(rq->destination.get()))
        {
            dest->free_buffer();
        }
    }
    return IOStatusComponent::setStatus(status);
}

using RAMRQPtr = skr::SObjectPtr<RAMIORequest>;

} // namespace io
} // namespace skr