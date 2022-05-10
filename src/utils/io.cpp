#include "utils/io.h"
#include "utils/io.hpp"
#include <ftl/task_counter.h>
#include "platform/memory.h"

bool skr_async_io_request_t::is_ready() const
{
    return SKR_ASYNC_IO_STATUS_OK == skr_atomic32_load_acquire(&status);
}

void skr::io::RAM::initialize()
{
}

void skr::io::RAM::request(skr_vfs_t* vfs, const char8_t* path,
uint8_t* bytes, uint64_t offset, uint64_t size,
skr_async_io_request_t* request)
{
}

// C API
void skr_io_ram_initialize()
{
    return skr::io::RAM::initialize();
}

void skr_io_ram_request(skr_vfs_t* vfs,
const char8_t* path, uint8_t* bytes,
uint64_t offset, uint64_t size, skr_async_io_request_t* request)
{
    return skr::io::RAM::request(vfs, path, bytes, offset, size, request);
}