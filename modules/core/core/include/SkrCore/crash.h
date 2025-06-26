#pragma once
#include "SkrBase/types.h"
#ifdef __cplusplus
    #include "SkrOS/thread.h"
    #include <SkrContainersDef/span.hpp>
    #include <SkrContainersDef/vector.hpp>

extern "C" {
#endif

enum
{
    // POSIX Signals
    kCrashCodeAbort          = 1, // abort()
    kCrashCodeInterrupt      = 2, // Ctrl-C
    kCrashCodeKill           = 3, // kill
    kCrashCodeDividedByZero  = 4, // divide by zero
    kCrashCodeIllInstruction = 5, // illegal instruction
    kCrashCodeSegFault       = 6, // segmentation fault
                                  // System Exceptions
    kCrashCodeStackOverflow = 7,  // Windows
    kCrashCodeTerminate     = 8,  //  (not catch) Windows/C++
    kCrashCodeUnexpected    = 9,  // (throw mismatch) Windows/C++
    kCrashCodeUnhandled     = 10, // (unhandled exception) Windows
    kCrashCodePureVirtual   = 12, // (pure-virtual call) Windows
    kCrashCodeOpNewError    = 13, // (new fatal) Windows
    kCrashCodeInvalidParam  = 14, // (invalid parameter) Windows
    kCrashCodeCount
};
typedef int32_t             CrashTerminateCode;
SKR_CORE_API const char8_t* skr_crash_code_string(CrashTerminateCode code) SKR_NOEXCEPT;

typedef struct SCrashHandler SCrashHandler;
typedef struct SCrashContext {
    CrashTerminateCode reason;
    void*              usr_data;
    bool               continue_execution;
#ifdef _WIN32
    struct _EXCEPTION_POINTERS* exception_pointers;
#endif
} SCrashContext;

typedef int (*SProcCrashCallback)(struct SCrashContext* context, void* usr_data);

typedef struct SCrashHandler* SCrashHandlerId;

SKR_CORE_API SCrashHandlerId skr_initialize_crash_handler() SKR_NOEXCEPT;
#ifdef SKR_PLAT_WINDOWS
SKR_CORE_API void skr_crash_handler_set_dump_file(const char8_t* dump_file) SKR_NOEXCEPT;
#endif
SKR_CORE_API SCrashHandlerId skr_crash_handler_get() SKR_NOEXCEPT;
SKR_CORE_API void            skr_crash_handler_add_callback(SCrashHandlerId handler, SProcCrashCallback callback, void* usr_data) SKR_NOEXCEPT;
SKR_CORE_API void            skr_finalize_crash_handler() SKR_NOEXCEPT;

#ifdef __cplusplus
}

typedef struct SCrashHandler {
    virtual ~SCrashHandler() SKR_NOEXCEPT = default;
    virtual bool Initialize() SKR_NOEXCEPT;
    virtual bool Finalize() SKR_NOEXCEPT;

    virtual bool SetProcessSignalHandlers() SKR_NOEXCEPT;
    virtual bool UnsetProcessSignalHandlers() SKR_NOEXCEPT;
    virtual bool SetThreadSignalHandlers() SKR_NOEXCEPT;
    virtual bool UnsetThreadSignalHandlers() SKR_NOEXCEPT;

    struct CallbackWrapper {
        SProcCrashCallback callback;
        void*              usr_data;
    };
    template <typename F>
    void visit_callbacks(F&& f) SKR_NOEXCEPT
    {
        SMutexLock _(callbacks_lock);
        for (auto& cb : callbacks)
        {
            f(cb);
        }
    }
    void add_callback(CallbackWrapper callback) SKR_NOEXCEPT;

protected:
    virtual void terminateProcess(int32_t code = 1) SKR_NOEXCEPT;

    virtual void beforeHandlingSignal(CrashTerminateCode code) SKR_NOEXCEPT;
    template <typename F>
    void                   handleFunction(F&& f, CrashTerminateCode code);
    virtual SCrashContext* getCrashContext(CrashTerminateCode reason) SKR_NOEXCEPT { return nullptr; }

    int        crashSetErrorMsg(const char8_t* pszErrorMsg) SKR_NOEXCEPT;
    void       crashLock() SKR_NOEXCEPT { skr_mutex_acquire(&crash_lock); }
    void       crashUnlock() SKR_NOEXCEPT { skr_mutex_release(&crash_lock); }
    SMutex     crash_lock;
    const bool kContinue = false;

    static void SigabrtHandler(int);
    static void SigfpeHandler(int, int subcode);
    static void SigintHandler(int);
    static void SigillHandler(int);
    static void SigsegvHandler(int);
    static void SigtermHandler(int);

    void(__cdecl* prevSigABRT)(int); // Previous SIGABRT handler.
    void(__cdecl* prevSigINT)(int);  // Previous SIGINT handler.
    void(__cdecl* prevSigTERM)(int); // Previous SIGTERM handler.

    void(__cdecl* prevSigFPE)(int);  // Previous FPE handler
    void(__cdecl* prevSigILL)(int);  // Previous SIGILL handler
    void(__cdecl* prevSigSEGV)(int); // Previous illegal storage access handler

    skr_guid_t guid;

    skr::Vector<CallbackWrapper> callbacks;
    SMutex                       callbacks_lock;
} SCrashHandler;

template <typename F>
void SCrashHandler::handleFunction(F&& f, CrashTerminateCode code)
{
    beforeHandlingSignal(code);

    auto& this_ = *skr_crash_handler_get();
    // Acquire lock to avoid other threads (if exist) to crash while we	are inside.
    this_.crashLock();
    f();
    // Terminate process
    if (!kContinue)
    {
        this_.terminateProcess(code);
    }
    // Free lock
    this_.crashUnlock();
}
#endif