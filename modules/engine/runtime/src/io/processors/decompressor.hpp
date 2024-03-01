#pragma once
#include "SkrRT/io/io.h"
#include "../common/processors.hpp"

namespace skr { template <typename Artifact> struct IFuture; struct JobQueue; }

namespace skr {
namespace io {

template<typename I = IIORequestProcessor>
struct TaskDecompressorBase : public IIODecompressor<I>
{
    IO_RESOLVER_OBJECT_BODY
public:
    TaskDecompressorBase() SKR_NOEXCEPT 
    {
        init_counters();
    }
    virtual ~TaskDecompressorBase() SKR_NOEXCEPT {}
};

} // namespace io
} // namespace skr