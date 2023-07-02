#pragma once
#include "misc/log/log_pattern.hpp"

namespace skr {
namespace log {

struct LogSink
{
    virtual ~LogSink() SKR_NOEXCEPT = default;

    skr_guid_t get_pattern() const SKR_NOEXCEPT { return pattern_; }

    virtual void sink(const LogEvent& event, skr::string_view content) SKR_NOEXCEPT;

protected:
    skr_guid_t pattern_ = LogConstants::kDefaultPatternId;
};   

} // namespace log
} // namespace skr