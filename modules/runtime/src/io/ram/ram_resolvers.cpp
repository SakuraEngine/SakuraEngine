#include "../common/io_resolver.hpp"

#include "ram_request.hpp"
#include "ram_buffer.hpp"

namespace skr {
namespace io {

struct VFSFileResolver : public IORequestResolverBase
{
    virtual void resolve(IORequestId request) SKR_NOEXCEPT
    {
        auto rq = skr::static_pointer_cast<RAMIORequest>(request);
        SKR_ASSERT(rq->vfs);
        if (!rq->file)
        {
            rq->file = skr_vfs_fopen(rq->vfs, rq->path.u8_str(), SKR_FM_READ_BINARY, SKR_FILE_CREATION_OPEN_EXISTING);
        }
    }
};

struct AllocateIOBufferResolver : public IORequestResolverBase
{
    virtual void resolve(IORequestId request) SKR_NOEXCEPT
    {
        ZoneScopedNC("IOBufferAllocate", tracy::Color::BlueViolet);
        auto rq = skr::static_pointer_cast<RAMIORequest>(request);
        auto buf = skr::static_pointer_cast<RAMIOBuffer>(rq->destination);
        // deal with 0 block size
        for (auto& block : rq->blocks)
        {
            if (!block.size)
            {
                block.size = rq->get_fsize() - block.offset;
            }
            if (!buf->size)
            {
                buf->size += block.size;
            }
        }
        // allocate
        if (!buf->bytes)
        {
            if (!buf->size)
            {
                SKR_ASSERT(0 && "invalid destination size");
            }
            buf->allocate_buffer(buf->size);
        }
    }
};

struct ChunkingVFSReadResolver : public IORequestResolverBase
{
    ChunkingVFSReadResolver(uint64_t chunk_size) : chunk_size(chunk_size) {}
    virtual void resolve(IORequestId request) SKR_NOEXCEPT
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
    const uint64_t chunk_size = 256 * 1024;
};

IORequestResolverId create_vfs_file_resolver() SKR_NOEXCEPT
{ 
    return SObjectPtr<VFSFileResolver>::Create();
}

IORequestResolverId create_vfs_buffer_resolver() SKR_NOEXCEPT
{
    return SObjectPtr<AllocateIOBufferResolver>::Create();
}

IORequestResolverId create_chunking_resolver(uint64_t chunk_size) SKR_NOEXCEPT
{
    return SObjectPtr<ChunkingVFSReadResolver>::Create(chunk_size);
}

} // namespace io
} // namespace skr