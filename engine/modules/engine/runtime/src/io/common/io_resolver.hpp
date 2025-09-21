#pragma once
#include "pool.hpp"
#include "SkrRuntime/io/io.h"

namespace skr {
namespace io {
struct RunnerBase;

struct IORequestResolverBase : public IIORequestResolver
{
    SKR_RC_IMPL(override)
    SKR_RC_DELETER_IMPL_DEFAULT(override)
};

struct VFSFileResolver final : public IORequestResolverBase
{
    void resolve(SkrAsyncServicePriority priority, IOBatchId batch, IORequestId request) SKR_NOEXCEPT;
};

} // namespace io
} // namespace skr