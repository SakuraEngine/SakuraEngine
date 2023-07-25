#include "../../pch.hpp"
#include "ram_resolvers.hpp"
#include "ram_request.hpp"
#include "ram_buffer.hpp"

namespace skr {
namespace io {

void AllocateIOBufferResolver::resolve(SkrAsyncServicePriority priority, IORequestId request) SKR_NOEXCEPT
{
    ZoneScopedNC("IOBuffer::Allocate", tracy::Color::BlueViolet);
    auto rq = skr::static_pointer_cast<RAMRequestMixin>(request);
    auto buf = skr::static_pointer_cast<RAMIOBuffer>(rq->destination);
    auto pFiles = io_component<FileSrcComponent>(rq.get());
    // deal with 0 block size
    if (auto pComp = io_component<BlocksComponent>(rq.get()))
    {
        for (auto& block : pComp->blocks)
        {
            if (block.size == 0)
            {
                block.size = pFiles->get_fsize() - block.offset;
            }
            if (buf->size == 0)
            {
                buf->size += block.size;
            }
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
    uint64_t total = 0;
    if (auto pBlocks = io_component<BlocksComponent>(request.get()))
    {
        for (auto& block : pBlocks->get_blocks())
            total += block.size;
    }
    uint64_t chunk_count = total / chunk_size;
    if (chunk_count > 2)
    {
        if (auto pComp = io_component<BlocksComponent>(request.get()))
        {
            auto bks = pComp->blocks;
            pComp->reset_blocks();
            pComp->blocks.reserve(chunk_count);
            for (auto& block : bks)
            {
                uint64_t acc_size = block.size;
                uint64_t acc_offset = block.offset;
                while (acc_size > chunk_size)
                {
                    pComp->add_block({acc_offset, chunk_size});
                    acc_offset += chunk_size;
                    acc_size -= chunk_size;
                }
                pComp->get_blocks().back().size += acc_size;
            }
        }
    }
}

} // namespace io
} // namespace skr