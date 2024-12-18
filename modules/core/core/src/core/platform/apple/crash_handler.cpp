#include "../unix/crash_handler.hpp"
#include "SkrCore/process.h"
#include "SkrCore/crash.h"

#include "SkrContainers/string.hpp"

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

    skr::String app_name;
};
MacCrashHandler mac_crash_handler;
}

extern "C"
{
SKR_RUNTIME_API SCrashHandlerId skr_initialize_crash_handler() SKR_NOEXCEPT
{
    auto& this_ = ::mac_crash_handler;
    this_.Initialize();
    return &this_;
}

SKR_RUNTIME_API SCrashHandlerId skr_crash_handler_get() SKR_NOEXCEPT
{
    return &::mac_crash_handler;
}

SKR_RUNTIME_API void skr_finalize_crash_handler() SKR_NOEXCEPT
{
    auto& this_ = ::mac_crash_handler;
    this_.Finalize();
}
}