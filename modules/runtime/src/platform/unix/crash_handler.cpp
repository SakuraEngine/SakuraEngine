#include "../../pch.hpp"
#include "misc/types.h"
#include "platform/thread.h"
#include "platform/process.h"
#include "platform/crash.h"

#include "containers/string.hpp"

extern "C"
{
    
RUNTIME_API SCrashHandlerId skr_initialize_crash_handler() SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return nullptr;
}

RUNTIME_API SCrashHandlerId skr_crash_handler_get() SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
    return nullptr;
}

RUNTIME_API void skr_crash_handler_add_callback(SCrashHandlerId handler, SProcCrashCallback callback, void* usr_data) SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
}

RUNTIME_API void skr_finalize_crash_handler() SKR_NOEXCEPT
{
    SKR_UNIMPLEMENTED_FUNCTION();
}

}