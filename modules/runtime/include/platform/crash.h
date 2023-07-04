#pragma once
#include "platform/configure.h"
#include "platform/thread.h"

#ifdef __cplusplus
#include "misc/types.h"

typedef struct SCrashHandler {
    virtual ~SCrashHandler() SKR_NOEXCEPT = default;
    virtual bool Initialize() SKR_NOEXCEPT;
    virtual bool Finalize() SKR_NOEXCEPT;

    virtual bool SetProcessSignalHandlers() SKR_NOEXCEPT;
    virtual bool UnsetProcessSignalHandlers() SKR_NOEXCEPT;
    virtual bool SetThreadSignalHandlers() SKR_NOEXCEPT;
    virtual bool UnsetThreadSignalHandlers() SKR_NOEXCEPT;

protected:
    virtual void terminateProcess(int32_t code = 1) SKR_NOEXCEPT;

    template<typename F>
    void handleFunction(F&& f);
    
    int crashSetErrorMsg(const char8_t* pszErrorMsg) SKR_NOEXCEPT;
    void crashLock() SKR_NOEXCEPT { skr_mutex_acquire(&crash_lock); }
    void crashUnlock() SKR_NOEXCEPT { skr_mutex_release(&crash_lock); }
    SMutex crash_lock;
    const bool kContinue = false;

    static void SigabrtHandler(int);
    static void SigfpeHandler(int, int subcode);
    static void SigintHandler(int);
    static void SigillHandler(int);
    static void SigsegvHandler(int);
    static void SigtermHandler(int);

    void (__cdecl *prevSigABRT)(int); // Previous SIGABRT handler.  
    void (__cdecl *prevSigINT)(int);  // Previous SIGINT handler.
    void (__cdecl *prevSigTERM)(int); // Previous SIGTERM handler.

    void (__cdecl *prevSigFPE)(int);   // Previous FPE handler
    void (__cdecl *prevSigILL)(int);   // Previous SIGILL handler
    void (__cdecl *prevSigSEGV)(int);  // Previous illegal storage access handler

    skr_guid_t guid;
} SCrashHandler;
extern "C" {
#else
typedef struct SCrashHandler SCrashHandler;
#endif

typedef struct SCrashContext {
    void* usr_data;
    bool continue_execution;
#ifdef _WIN32
    struct _EXCEPTION_POINTERS* exception_pointers;
#endif
} SCrashContext;

typedef int(*SProcCrashCallback)(struct SCrashContext* context);

typedef struct SCrashHandler* SCrashHandlerId;

RUNTIME_API SCrashHandlerId skr_initialize_crash_handler() SKR_NOEXCEPT;
RUNTIME_API SCrashHandlerId skr_crash_handler_get() SKR_NOEXCEPT;
RUNTIME_API void skr_crash_handler_add_callback(SCrashHandlerId handler, SProcCrashCallback callback, void* usr_data) SKR_NOEXCEPT;
RUNTIME_API void skr_finalize_crash_handler() SKR_NOEXCEPT;

#ifdef __cplusplus
}
template<typename F>
void SCrashHandler::handleFunction(F&& f)
{
    auto& this_ = *skr_crash_handler_get();
    // Acquire lock to avoid other threads (if exist) to crash while we	are inside.
    this_.crashLock();

    f();

    // Terminate process
    if (kContinue)
    {
        this_.terminateProcess();
    }
    // Free lock
    this_.crashUnlock();
}
#endif