#pragma once
#include "ram_resolvers.hpp"
#include "ram_request.hpp"
#include "ram_buffer.hpp"

namespace skr {
namespace io {

void AllocateIOBufferResolver::resolve(SkrAsyncServicePriority priority, IORequestId request) SKR_NOEXCEPT
{
    ZoneScopedNC("IOBuffer::Allocate", tracy::Color::BlueViolet);
    auto rq = skr::static_pointer_cast<RAMIORequest>(request);
    auto buf = skr::static_pointer_cast<RAMIOBuffer>(rq->destination);
    // deal with 0 block size
    for (auto& block : rq->blocks)
    {
        if (block.size == 0)
        {
            block.size = rq->get_fsize() - block.offset;
        }
        if (buf->size == 0)
        {
            buf->size += block.size;
        }
    }
    // allocate
    if (buf->bytes == nullptr)
    {
        if (buf->size == 0)
        {
            SKR_ASSERT(0 && "invalid destination size");
        }
        buf->allocate_buffer(buf->size);
    }
}

ChunkingVFSReadResolver::ChunkingVFSReadResolver(uint64_t chunk_size) SKR_NOEXCEPT
    : chunk_size(chunk_size) 
{
    
}

void ChunkingVFSReadResolver::resolve(SkrAsyncServicePriority priority,IORequestId request) SKR_NOEXCEPT
{
    ZoneScopedN("IORequestChunking");
    auto rq = skr::static_pointer_cast<RAMIORequest>(request);
    uint64_t total = 0;
    for (auto& block : rq->get_blocks())
        total += block.size;
    uint64_t chunk_count = total / chunk_size;
    if (chunk_count > 2)
    {
        auto bks = rq->blocks;
        rq->reset_blocks();
        rq->blocks.reserve(chunk_count);
        for (auto& block : bks)
        {
            uint64_t acc_size = block.size;
            uint64_t acc_offset = block.offset;
            while (acc_size > chunk_size)
            {
                rq->add_block({acc_offset, chunk_size});
                acc_offset += chunk_size;
                acc_size -= chunk_size;
            }
            rq->get_blocks().back().size += acc_size;
        }
    }
}

} // namespace io
} // namespace skr