#include "../../pch.hpp"
#include "../unix/crash_handler.hpp"
#include "platform/process.h"
#include "platform/crash.h"

#include "containers/string.hpp"

namespace 
{
struct MacCrashHandler : public SUnixCrashHandler
{
    MacCrashHandler() SKR_NOEXCEPT
        : SUnixCrashHandler()
    {

    }

    bool Initialize() SKR_NOEXCEPT
    {
        app_name = skr_get_current_process_name();
        return SUnixCrashHandler::Initialize();
    }

    skr::string app_name;
};
MacCrashHandler mac_crash_handler;
}

extern "C"
{
RUNTIME_API SCrashHandlerId skr_initialize_crash_handler() SKR_NOEXCEPT
{
    auto& this_ = ::mac_crash_handler;
    this_.Initialize();
    return &this_;
}

RUNTIME_API SCrashHandlerId skr_crash_handler_get() SKR_NOEXCEPT
{
    return &::mac_crash_handler;
}

RUNTIME_API void skr_finalize_crash_handler() SKR_NOEXCEPT
{
    auto& this_ = ::mac_crash_handler;
    this_.Finalize();
}
}