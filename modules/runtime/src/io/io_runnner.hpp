#pragma once
#include "async/service_thread.hpp"
#include "io_batch.hpp"

namespace skr {
namespace io {

struct RunnerBase : public skr::ServiceThread
{

};

} // namespace io
} // namespace skr