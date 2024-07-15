#include "SkrCore/log.h"
#include "SkrCore/crash.h"
#include "SkrOS/thread.h"
#include "SkrContainersDef/hashmap.hpp"
#include "SkrContainersDef/string.hpp"

#include <signal.h>

skr::ParallelFlatHashMap<uint64_t, skr::String> error_map;

void SCrashHandler::terminateProcess(int32_t code) SKR_NOEXCEPT
{
    ::exit(code);
}

// Sets the last error message (for the caller thread).
int SCrashHandler::crashSetErrorMsg(const char8_t* pszErrorMsg) SKR_NOEXCEPT
{
    const auto tid = skr_current_thread_id();
    error_map[tid] = pszErrorMsg;
    return 0;
}

bool SCrashHandler::Initialize() SKR_NOEXCEPT
{
    prevSigABRT = NULL;
    prevSigINT  = NULL;
    prevSigTERM = NULL;
    prevSigFPE  = NULL;
    prevSigILL  = NULL;
    prevSigSEGV = NULL;

    skr_make_guid(&guid);
    skr_init_mutex_recursive(&crash_lock);
    skr_init_mutex_recursive(&callbacks_lock);
    // set process exception handlers
    if (bool phdls = SetProcessSignalHandlers(); !phdls)
    {
        crashSetErrorMsg(u8"Failed to set process exception handlers.");
        return false;
    }
    // set thread exception handlers
    if (bool thdls = SetThreadSignalHandlers(); !thdls)
    {
        crashSetErrorMsg(u8"Failed to set thread exception handlers.");
        return false;
    }
    return true;
}

bool SCrashHandler::Finalize() SKR_NOEXCEPT
{
    UnsetProcessSignalHandlers();
    UnsetThreadSignalHandlers();

    skr_destroy_mutex(&callbacks_lock);
    skr_destroy_mutex(&crash_lock);
    return true;
}

void SCrashHandler::SigabrtHandler(int code)
{
    auto&      this_  = *skr_crash_handler_get();
    const auto reason = kCrashCodeAbort;
    this_.handleFunction([&]() {
        this_.visit_callbacks([&](const CallbackWrapper& wrapper) {
            auto ctx = this_.getCrashContext(reason);
            wrapper.callback(ctx, wrapper.usr_data);
        });
    },
                         reason);
}

void SCrashHandler::SigintHandler(int code)
{
    auto&      this_  = *skr_crash_handler_get();
    const auto reason = kCrashCodeInterrupt;
    this_.handleFunction([&]() {
        this_.visit_callbacks([&](const CallbackWrapper& wrapper) {
            auto ctx = this_.getCrashContext(reason);
            wrapper.callback(ctx, wrapper.usr_data);
        });
    },
                         reason);
}

void SCrashHandler::SigtermHandler(int code)
{
    auto&      this_  = *skr_crash_handler_get();
    const auto reason = kCrashCodeKill;
    this_.handleFunction([&]() {
        this_.visit_callbacks([&](const CallbackWrapper& wrapper) {
            auto ctx = this_.getCrashContext(reason);
            wrapper.callback(ctx, wrapper.usr_data);
        });
    },
                         reason);
}

void SCrashHandler::SigfpeHandler(int code, int subcode)
{
    auto&      this_  = *skr_crash_handler_get();
    const auto reason = kCrashCodeDividedByZero;
    this_.handleFunction([&]() {
        this_.visit_callbacks([&](const CallbackWrapper& wrapper) {
            auto ctx = this_.getCrashContext(reason);
            wrapper.callback(ctx, wrapper.usr_data);
        });
    },
                         reason);
}

void SCrashHandler::SigillHandler(int code)
{
    auto&      this_  = *skr_crash_handler_get();
    const auto reason = kCrashCodeIllInstruction;
    this_.handleFunction([&]() {
        this_.visit_callbacks([&](const CallbackWrapper& wrapper) {
            auto ctx = this_.getCrashContext(reason);
            wrapper.callback(ctx, wrapper.usr_data);
        });
    },
                         reason);
}

void SCrashHandler::SigsegvHandler(int code)
{
    auto&      this_  = *skr_crash_handler_get();
    const auto reason = kCrashCodeSegFault;
    this_.handleFunction([&]() {
        this_.visit_callbacks([&](const CallbackWrapper& wrapper) {
            auto ctx = this_.getCrashContext(reason);
            wrapper.callback(ctx, wrapper.usr_data);
        });
    },
                         reason);
}

bool SCrashHandler::SetProcessSignalHandlers() SKR_NOEXCEPT
{
    prevSigABRT = signal(SIGABRT, SigabrtHandler); // abort
    prevSigINT  = signal(SIGINT, SigintHandler);   // ctrl + c
    prevSigTERM = signal(SIGTERM, SigtermHandler); // kill
    return true;
}

bool SCrashHandler::UnsetProcessSignalHandlers() SKR_NOEXCEPT
{
    if (prevSigABRT != NULL)
        signal(SIGABRT, prevSigABRT);
    if (prevSigINT != NULL)
        signal(SIGINT, prevSigINT);
    if (prevSigTERM != NULL)
        signal(SIGTERM, prevSigTERM);
    return true;
}

bool SCrashHandler::SetThreadSignalHandlers() SKR_NOEXCEPT
{
    typedef void (*sigh)(int);
    prevSigFPE  = signal(SIGFPE, (sigh)SigfpeHandler); // divide by zero
    prevSigILL  = signal(SIGILL, SigillHandler);       // illegal instruction
    prevSigSEGV = signal(SIGSEGV, SigsegvHandler);     // segmentation fault
    return true;
}

bool SCrashHandler::UnsetThreadSignalHandlers() SKR_NOEXCEPT
{
    if (prevSigFPE != NULL)
        signal(SIGFPE, prevSigFPE);
    if (prevSigILL != NULL)
        signal(SIGILL, prevSigILL);
    if (prevSigSEGV != NULL)
        signal(SIGSEGV, prevSigSEGV);
    return true;
}

void SCrashHandler::beforeHandlingSignal(CrashTerminateCode code) SKR_NOEXCEPT
{
    skr_log_finalize_async_worker();
    skr_log_set_flush_behavior(SKR_LOG_FLUSH_BEHAVIOR_IMMEDIATE);
}

void SCrashHandler::add_callback(SCrashHandler::CallbackWrapper callback) SKR_NOEXCEPT
{
    SMutexLock _(callbacks_lock);
    callbacks.add(callback);
}

extern "C" {

SKR_CORE_API const char8_t* skr_crash_code_string(CrashTerminateCode code) SKR_NOEXCEPT
{
#define SKR_CCODE_TRANS(code) \
    case kCrashCode##code:    \
        return (const char8_t*)#code;
    switch (code)
    {
        SKR_CCODE_TRANS(Abort);
        SKR_CCODE_TRANS(Interrupt);
        SKR_CCODE_TRANS(Kill);
        SKR_CCODE_TRANS(DividedByZero);
        SKR_CCODE_TRANS(IllInstruction);
        SKR_CCODE_TRANS(SegFault);
        SKR_CCODE_TRANS(StackOverflow);
        SKR_CCODE_TRANS(Terminate);
        SKR_CCODE_TRANS(Unexpected);
        SKR_CCODE_TRANS(Unhandled);
        SKR_CCODE_TRANS(PureVirtual);
        SKR_CCODE_TRANS(OpNewError);
        SKR_CCODE_TRANS(InvalidParam);
    }
    return u8"";
#undef SKR_CCODE_TRANS
}

SKR_CORE_API void skr_crash_handler_add_callback(SCrashHandlerId handler, SProcCrashCallback callback, void* usr_data) SKR_NOEXCEPT
{
    return handler->add_callback({ callback, usr_data });
}
}