#pragma once
#include "../common/io_resolver.hpp"

namespace skr {
namespace io {

struct DStorageFileResolver final : public IORequestResolverBase
{
    void resolve(SkrAsyncServicePriority priority, IORequestId request) SKR_NOEXCEPT;
};

} // namespace io
} // namespace skr