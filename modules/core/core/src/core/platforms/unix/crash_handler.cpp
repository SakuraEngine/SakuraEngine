#include "../unix/crash_handler.hpp"

SUnixCrashHandler::SUnixCrashHandler() SKR_NOEXCEPT
    : SCrashHandler()
{

}

SUnixCrashHandler::~SUnixCrashHandler() SKR_NOEXCEPT
{

}

#ifndef UNIX_CRASH_HANDLER_IMPLEMENTED
namespace 
{
    SUnixCrashHandler unix_crash_handler;
}
extern "C"
{
SKR_CORE_API SCrashHandlerId skr_initialize_crash_handler() SKR_NOEXCEPT
{
    auto& this_ = ::unix_crash_handler;
    this_.Initialize();
    return &this_;
}

SKR_CORE_API SCrashHandlerId skr_crash_handler_get() SKR_NOEXCEPT
{
    return &::unix_crash_handler;
}

SKR_CORE_API void skr_finalize_crash_handler() SKR_NOEXCEPT
{
    auto& this_ = ::unix_crash_handler;
    this_.Finalize();
}
}
#endif