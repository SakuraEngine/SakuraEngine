#pragma once
#include "pool.hpp"
#include "SkrRT/io/io.h"

namespace skr {
namespace io {
struct RunnerBase;

struct IORequestResolverBase : public IIORequestResolver
{
    IO_RC_OBJECT_BODY
};

struct VFSFileResolver final : public IORequestResolverBase
{
    void resolve(SkrAsyncServicePriority priority, IOBatchId batch, IORequestId request) SKR_NOEXCEPT;
};

} // namespace io
} // namespace skr