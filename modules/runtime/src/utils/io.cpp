#include "utils/io.h"
#include "utils/io.hpp"


bool skr_async_request_t::is_ready() const SKR_NOEXCEPT
{
    return get_status() == SKR_ASYNC_IO_STATUS_OK;
}
bool skr_async_request_t::is_enqueued() const SKR_NOEXCEPT
{
    return get_status() == SKR_ASYNC_IO_STATUS_ENQUEUED;
}
bool skr_async_request_t::is_cancelled() const SKR_NOEXCEPT
{
    return get_status() == SKR_ASYNC_IO_STATUS_CANCELLED;
}
bool skr_async_request_t::is_ram_loading() const SKR_NOEXCEPT
{
    return get_status() == SKR_ASYNC_IO_STATUS_RAM_LOADING;
}
bool skr_async_request_t::is_vram_loading() const SKR_NOEXCEPT
{
    return get_status() == SKR_ASYNC_IO_STATUS_VRAM_LOADING;
}

SkrAsyncIOStatus skr_async_request_t::get_status() const SKR_NOEXCEPT
{
    return (SkrAsyncIOStatus)skr_atomic32_load_acquire(&status);
}
