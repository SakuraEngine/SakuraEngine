#include "common/io_resolver.hpp"

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
    return (ESkrIOStage)skr_atomicu32_load_acquire(&status);
}

namespace skr {
namespace io {

const char* kIOPoolObjectsMemoryName = "I/O PoolObjects";
const char* kIOConcurrentQueueName = "IOConcurrentQueue";

IIORequestResolver::~IIORequestResolver() SKR_NOEXCEPT
{

}

void IIORequestResolver::resolve(IORequestId request) SKR_NOEXCEPT
{
    (void)request;
}

IIOBatchBuffer::~IIOBatchBuffer() SKR_NOEXCEPT
{

}

IIORequestResolverChain::~IIORequestResolverChain() SKR_NOEXCEPT
{

}

IIOBatchProcessorChain::~IIOBatchProcessorChain() SKR_NOEXCEPT
{

}

IIOBatchProcessor::~IIOBatchProcessor() SKR_NOEXCEPT
{
    
}

IIOReader::~IIOReader() SKR_NOEXCEPT
{
    
}

IIODecompressor::~IIODecompressor() SKR_NOEXCEPT
{
    
}

SObjectPtr<IIORequestResolverChain> IIORequestResolverChain::Create(IORequestResolverId resolver) SKR_NOEXCEPT
{
    auto chain = SObjectPtr<IORequestResolverChain>::Create(resolver);
    return chain;
}

} // namespace io
} // namespace skr