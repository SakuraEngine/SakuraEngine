#pragma once
#include "../common/io_batch.hpp"
#include "ram_request.hpp"

namespace skr {
namespace io {

struct RAMIOBatch : public IOBatchBase
{
    RAMIOBatch(ISmartPool<IIOBatch>* pool, uint64_t seq, uint64_t n)
        : IOBatchBase(pool, seq)
    {
        reserve(n);
    }

    IOResultId add_request(IORequestId request, skr_io_future_t* future) SKR_NOEXCEPT;
};
using RAMBatchPtr = skr::SObjectPtr<RAMIOBatch>;

} // namespace io
} // namespace skr