#pragma once
#include "SkrRT/io/ram_io.hpp"
#include "io/common/io_batch.hpp"

namespace skr {
namespace io {

struct RAMIOBatch : public IOBatchBase
{
    RAMIOBatch(ISmartPool<IIOBatch>* pool, IIOService* service, uint64_t seq, uint64_t n)
        : IOBatchBase(pool, service, seq)
    {
        reserve(n);
    }

    IOResultId add_request(IORequestId request, skr_io_future_t* future) SKR_NOEXCEPT;
    void add_request(IORequestId request, RAMIOBufferId dest, skr_io_future_t* future) SKR_NOEXCEPT;
};
using RAMBatchPtr = skr::SObjectPtr<RAMIOBatch>;

} // namespace io
} // namespace skr