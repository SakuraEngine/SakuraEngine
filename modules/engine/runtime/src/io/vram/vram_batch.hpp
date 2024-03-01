#pragma once
#include "../common/io_batch.hpp"

namespace skr {
namespace io {

struct VRAMIOBatch : public IOBatchBase
{
    VRAMIOBatch(ISmartPool<IIOBatch>* pool, IIOService* service, uint64_t seq, uint64_t n) SKR_NOEXCEPT;
    IOResultId add_request(IORequestId request, skr_io_future_t* future) SKR_NOEXCEPT;
};
using VRAMBatchPtr = skr::SObjectPtr<VRAMIOBatch>;

} // namespace io
} // namespace skr