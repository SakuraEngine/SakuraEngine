#pragma once
#include "SkrRT/io/io.h"
#include "SkrRT/platform/guid.hpp"
#include "../components/component.hpp"
#include <EASTL/fixed_vector.h>

namespace skr {
namespace io {

template <>
struct CID<struct BlocksComponent> 
{
    static constexpr skr_guid_t Get();
};
struct BlocksComponent : public IORequestComponent
{
    BlocksComponent(IIORequest* const request) SKR_NOEXCEPT;
    
    skr::span<skr_io_block_t> get_blocks() SKR_NOEXCEPT 
    { 
        return blocks;
    }

    void add_block(const skr_io_block_t& block) SKR_NOEXCEPT 
    {
        blocks.emplace_back(block);
    }
    
    void reset_blocks() SKR_NOEXCEPT { blocks.clear(); }
    
    eastl::fixed_vector<skr_io_block_t, 1> blocks;
};

template <>
struct CID<struct CompressedBlocksComponent> 
{
    static constexpr skr_guid_t Get();
};
struct CompressedBlocksComponent : public IORequestComponent
{
    CompressedBlocksComponent(IIORequest* const request) SKR_NOEXCEPT;
    
    skr::span<skr_io_compressed_block_t> get_compressed_blocks() SKR_NOEXCEPT 
    { 
        SKR_UNIMPLEMENTED_FUNCTION();
        return {}; 
    }

    void add_compressed_block(const skr_io_block_t& block) SKR_NOEXCEPT 
    {  
        SKR_UNIMPLEMENTED_FUNCTION();
    }

    void reset_compressed_blocks() SKR_NOEXCEPT 
    {
        SKR_UNIMPLEMENTED_FUNCTION();
    }
};

constexpr skr_guid_t CID<struct BlocksComponent>::Get()
{
    using namespace skr::guid::literals;
    return u8"5c630f52-ec5b-4e6d-8d52-6e7933bd588d"_guid;
} 

constexpr skr_guid_t CID<struct CompressedBlocksComponent>::Get()
{
    using namespace skr::guid::literals;
    return u8"c4554100-4810-4372-817a-2c72eebcb377"_guid;
} 

} // namespace io
} // namespace skr