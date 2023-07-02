#pragma once
#include "log_queue.hpp"
#include "log_worker.hpp"

namespace skr {
namespace log {

struct Logger
{
    Logger() SKR_NOEXCEPT
    {

    }
    
private:
    LogQueue queue_;
    LogWorker worker_;
};

} } // namespace skr::log