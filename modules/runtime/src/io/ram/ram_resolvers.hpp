#pragma once
#include "../common/io_resolver.hpp"

namespace skr {
namespace io {

struct AllocateIOBufferResolver final : public IORequestResolverBase
{
    void resolve(SkrAsyncServicePriority priority, IOBatchId batch, IORequestId request) SKR_NOEXCEPT;
};

struct ChunkingVFSReadResolver : public IORequestResolverBase
{
    ChunkingVFSReadResolver(uint64_t chunk_size) SKR_NOEXCEPT;
    void resolve(SkrAsyncServicePriority priority, IOBatchId batch, IORequestId request) SKR_NOEXCEPT;
    const uint64_t chunk_size = 256 * 1024;
};

} // namespace io
} // namespace skr