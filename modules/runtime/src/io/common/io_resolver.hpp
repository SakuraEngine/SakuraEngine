#pragma once
#include "io/io.h"
#include "pool.hpp"

namespace skr {
namespace io {
struct RunnerBase;

struct IORequestResolverBase : public IIORequestResolver
{
    IO_RC_OBJECT_BODY
public:

};

} // namespace io
} // namespace skr