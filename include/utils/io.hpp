#pragma once
#include "io.h"

namespace skr
{
namespace io
{
typedef class RUNTIME_API RAMService
{
public:
    [[nodiscard]] static RAMService* create(const skr_ram_io_service_desc_t* desc) RUNTIME_NOEXCEPT;
    static void destroy(RAMService* service) RUNTIME_NOEXCEPT;

    // we do not lock an ioService to a single vfs
    // but for better bandwidth use and easier profiling
    // it's recommended to make a unique relevance between ioService & vfs
    virtual void request(skr_vfs_t*, const skr_ram_io_t* info, skr_async_io_request_t* async_request) RUNTIME_NOEXCEPT = 0;

    // try to cancel an enqueued request
    // returns false if the request is under LOADING status
    virtual bool try_cancel(skr_async_io_request_t* request) RUNTIME_NOEXCEPT = 0;

    // block & finish up all requests
    virtual void drain() RUNTIME_NOEXCEPT = 0;

    virtual ~RAMService() = default;
    RAMService() = default;
} RAMService;
} // namespace io
} // namespace skr