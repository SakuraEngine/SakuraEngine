#pragma once
#include "SkrRT/io/io.h"

SKR_DECLARE_TYPE_ID_FWD(skr::io, IRAMService, skr_io_ram_service)

typedef struct skr_ram_io_service_desc_t {
    const char8_t* name SKR_IF_CPP(= nullptr);
    uint32_t sleep_time SKR_IF_CPP(= SKR_ASYNC_SERVICE_SLEEP_TIME_MAX);
    skr_job_queue_id io_job_queue SKR_IF_CPP(= nullptr);
    skr_job_queue_id callback_job_queue SKR_IF_CPP(= nullptr);
    bool awake_at_request SKR_IF_CPP(= true);
    bool use_dstorage SKR_IF_CPP(= true);
} skr_ram_io_service_desc_t;

namespace skr {
namespace io {

// io flow
// 1. Enqueue requests(batches) to service
// 2. Sort raw requests
// 3. Resolve requests to pending raw request array
//  3.1 package parsing happens here.
//  3.2 you can resolve paths, open files, allocate buffers, etc.
// 4. Dispatch I/O blocks to drives (+allocate & cpy to raw)
// 5. Do uncompress works (+allocate & cpy to uncompressed)
// 6. Two kinds of callbacks are provided
//  6.1 inplace callbacks are executed in the I/O thread/workers
//  6.2 finish callbacks are polled & executed by usr threads
struct IRAMService;
using RAMServiceDescriptor = skr_ram_io_service_desc_t;

struct SKR_RUNTIME_API IRAMIOBuffer : public skr::IBlob
{
    virtual ~IRAMIOBuffer() SKR_NOEXCEPT;
};
using RAMIOBufferId = SObjectPtr<IRAMIOBuffer>;

struct SKR_RUNTIME_API IBlocksRAMRequest : public IIORequest
{
    virtual ~IBlocksRAMRequest() SKR_NOEXCEPT;

#pragma region BlocksComponent
    virtual skr::span<skr_io_block_t> get_blocks() SKR_NOEXCEPT = 0;
    virtual void add_block(const skr_io_block_t& block) SKR_NOEXCEPT = 0;
    virtual void reset_blocks() SKR_NOEXCEPT = 0;
#pragma endregion

#pragma region CompressedBlocksComponent
    virtual skr::span<skr_io_compressed_block_t> get_compressed_blocks() SKR_NOEXCEPT = 0;
    virtual void add_compressed_block(const skr_io_block_t& block) SKR_NOEXCEPT = 0;
    virtual void reset_compressed_blocks() SKR_NOEXCEPT = 0;
#pragma endregion
};
using BlocksRAMRequestId = SObjectPtr<IBlocksRAMRequest>;

struct SKR_RUNTIME_API IRAMService : public IIOService
{
    [[nodiscard]] static IRAMService* create(const RAMServiceDescriptor* desc) SKR_NOEXCEPT;
    static void destroy(IRAMService* service) SKR_NOEXCEPT;

    // open a request for filling
    [[nodiscard]] virtual BlocksRAMRequestId open_request() SKR_NOEXCEPT = 0;

    // start a request batch
    [[nodiscard]] virtual IOBatchId open_batch(uint64_t n) SKR_NOEXCEPT = 0;

    // submit a request
    [[nodiscard]] virtual RAMIOBufferId request(IORequestId request, IOFuture* future, SkrAsyncServicePriority priority = SKR_ASYNC_SERVICE_PRIORITY_NORMAL) SKR_NOEXCEPT = 0;
    
    // submit a batch
    virtual void request(IOBatchId request) SKR_NOEXCEPT = 0;

    virtual ~IRAMService() SKR_NOEXCEPT = default;
    IRAMService() SKR_NOEXCEPT = default;
};

} // namespace io
} // namespace skr