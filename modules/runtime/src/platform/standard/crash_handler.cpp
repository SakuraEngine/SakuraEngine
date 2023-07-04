#include "platform/crash.h"
#include "platform/thread.h"
#include "containers/hashmap.hpp"
#include "containers/string.hpp"

#include <signal.h>

skr::parallel_flat_hash_map<uint64_t, skr::string> error_map;

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
    prevSigINT = NULL;
    prevSigTERM = NULL;
    prevSigFPE = NULL;
    prevSigILL = NULL;
    prevSigSEGV = NULL;

    skr_make_guid(&guid);
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
    return true;
}

void SCrashHandler::SigabrtHandler(int code)
{
    auto& this_ = *skr_crash_handler_get();
	this_.handleFunction([&](){
        (void)code; // TODO: do something here...
    }, kCrashCodeAbort);
}

void SCrashHandler::SigintHandler(int code)
{
    auto& this_ = *skr_crash_handler_get();
	this_.handleFunction([&](){
        (void)code; // TODO: do something here...
    }, kCrashCodeInterrupt);
}

void SCrashHandler::SigtermHandler(int code)
{
    auto& this_ = *skr_crash_handler_get();
	this_.handleFunction([&](){
        (void)code; // TODO: do something here...
    }, kCrashCodeKill);
}

void SCrashHandler::SigfpeHandler(int code, int subcode)
{
    auto& this_ = *skr_crash_handler_get();
	this_.handleFunction([&](){
        (void)code; // TODO: do something here...
    }, kCrashCodeDividedByZero);
}

void SCrashHandler::SigillHandler(int code)
{
    auto& this_ = *skr_crash_handler_get();
	this_.handleFunction([&](){
        (void)code; // TODO: do something here...
    }, kCrashCodeIllInstruction);
}

void SCrashHandler::SigsegvHandler(int code)
{
    auto& this_ = *skr_crash_handler_get();
	this_.handleFunction([&](){
        (void)code; // TODO: do something here...
    }, kCrashCodeSegFault);
}

bool SCrashHandler::SetProcessSignalHandlers() SKR_NOEXCEPT
{
    prevSigABRT = signal(SIGABRT, SigabrtHandler);// abort
    prevSigINT = signal(SIGINT, SigintHandler);   // ctrl + c
    prevSigTERM = signal(SIGTERM, SigtermHandler);// kill
    return true;
}

bool SCrashHandler::UnsetProcessSignalHandlers() SKR_NOEXCEPT
{
    if(prevSigABRT != NULL)
        signal(SIGABRT, prevSigABRT);  
    if(prevSigINT != NULL)
        signal(SIGINT, prevSigINT);     
    if(prevSigTERM != NULL)
        signal(SIGTERM, prevSigTERM);   
    return true;
}

bool SCrashHandler::SetThreadSignalHandlers() SKR_NOEXCEPT
{
    typedef void (*sigh)(int);
    prevSigFPE = signal(SIGFPE, (sigh)SigfpeHandler); // divide by zero
    prevSigILL = signal(SIGILL, SigillHandler);       // illegal instruction
    prevSigSEGV = signal(SIGSEGV, SigsegvHandler);    // segmentation fault
    return true;
}

bool SCrashHandler::UnsetThreadSignalHandlers() SKR_NOEXCEPT
{
    if(prevSigFPE != NULL)
        signal(SIGFPE, prevSigFPE);
    if(prevSigILL != NULL)
        signal(SIGILL, prevSigILL);
    if(prevSigSEGV != NULL)
        signal(SIGSEGV, prevSigSEGV);
    return true;
}