#include "SkrRT/io/io.h"
#include "common/processors.hpp"

bool skr_io_future_t::is_ready() const SKR_NOEXCEPT
{
    return get_status() == SKR_IO_STAGE_COMPLETED;
}
bool skr_io_future_t::is_enqueued() const SKR_NOEXCEPT
{
    return get_status() == SKR_IO_STAGE_ENQUEUED;
}
bool skr_io_future_t::is_cancelled() const SKR_NOEXCEPT
{
    return get_status() == SKR_IO_STAGE_CANCELLED;
}
bool skr_io_future_t::is_loading() const SKR_NOEXCEPT
{
    return get_status() == SKR_IO_STAGE_LOADING;
}
ESkrIOStage skr_io_future_t::get_status() const SKR_NOEXCEPT
{
    return (ESkrIOStage)atomic_load_acquire(&status);
}

namespace skr {
namespace io {

const char* kIOPoolObjectsMemoryName = "io::objects";
const char* kIOConcurrentQueueName = "io::queue";

IIORequest::~IIORequest() SKR_NOEXCEPT
{
    
}

IIOProcessor::~IIOProcessor() SKR_NOEXCEPT
{

}

IIORequestProcessor::~IIORequestProcessor() SKR_NOEXCEPT
{

}

IIOBatchProcessor::~IIOBatchProcessor() SKR_NOEXCEPT
{
    
}

IIORequestResolver::~IIORequestResolver() SKR_NOEXCEPT
{

}

void IIORequestResolver::resolve(SkrAsyncServicePriority priority, IOBatchId batch, IORequestId request) SKR_NOEXCEPT
{

}

IIORequestResolverChain::~IIORequestResolverChain() SKR_NOEXCEPT
{

}

uint64_t IIOBatchProcessor::get_prefer_batch_size() const SKR_NOEXCEPT { return UINT64_MAX; }
// uint64_t IIOBatchProcessor::get_prefer_batch_count() const SKR_NOEXCEPT { return UINT64_MAX; }

template<>
IIOReader<IIORequestProcessor>::~IIOReader() SKR_NOEXCEPT {}
template<>
IIOReader<IIOBatchProcessor>::~IIOReader() SKR_NOEXCEPT {}
template<>
IIODecompressor<IIORequestProcessor>::~IIODecompressor() SKR_NOEXCEPT {}
template<>
IIODecompressor<IIOBatchProcessor>::~IIODecompressor() SKR_NOEXCEPT {}

SObjectPtr<IIORequestResolverChain> IIORequestResolverChain::Create(IORequestResolverId resolver) SKR_NOEXCEPT
{
    auto chain = SObjectPtr<IORequestResolverChain>::Create(resolver);
    return chain;
}

} // namespace io
} // namespace skr