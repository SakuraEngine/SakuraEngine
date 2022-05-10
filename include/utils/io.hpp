#pragma once
#include "io.h"
#include <ftl/task_scheduler.h>
#include "platform/thread.h"
#include <EASTL/unique_ptr.h>

namespace skr
{
namespace io
{
typedef struct RUNTIME_API RAM {
    static void initialize();
    static void request(skr_vfs_t* vfs,
    const char8_t* path, uint8_t* bytes,
    uint64_t offset, uint64_t size, skr_async_io_request_t* request);
    static eastl::unique_ptr<ftl::TaskScheduler> ramTaskScheduler;
} RAM;
} // namespace io
} // namespace skr