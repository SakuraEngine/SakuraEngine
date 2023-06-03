#include "ram/ram_request.hpp"

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

IIOReader::~IIOReader() SKR_NOEXCEPT
{
    
}

} // namespace io
} // namespace skr