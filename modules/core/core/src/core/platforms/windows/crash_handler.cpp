#include "./../../winheaders.h"

#include "SkrCore/process.h"
#include "SkrCore/crash.h"

#include "SkrContainersDef/string.hpp"

#include <signal.h>
#include <Dbghelp.h>

#pragma comment(lib, "dbghelp.lib")
#pragma comment(lib, "User32.lib")

namespace
{
static const char8_t* kDebugHelpDLLName = u8"dbghelp.dll";
struct WinCrashHandler : public SCrashHandler {
    WinCrashHandler() SKR_NOEXCEPT = default;
    virtual ~WinCrashHandler() SKR_NOEXCEPT;

    WinCrashHandler(WinCrashHandler const&)            = delete;
    WinCrashHandler& operator=(WinCrashHandler const&) = delete;

    bool Initialize() SKR_NOEXCEPT override;
    bool Finalize() SKR_NOEXCEPT override;
    void SetDumpFile(const char8_t* dumpFile) SKR_NOEXCEPT { dump_file = dumpFile;}
    bool SetProcessSignalHandlers() SKR_NOEXCEPT override;
    bool SetThreadSignalHandlers() SKR_NOEXCEPT override;
    bool UnsetProcessSignalHandlers() SKR_NOEXCEPT override;
    bool UnsetThreadSignalHandlers() SKR_NOEXCEPT override;
    void terminateProcess(int32_t code = 1) SKR_NOEXCEPT override
    {
        TerminateProcess(GetCurrentProcess(), code);
    }

    SCrashContext  ctx;
    SCrashContext* getCrashContext(CrashTerminateCode reason) SKR_NOEXCEPT override
    {
        ctx.reason = reason;
        return &ctx;
    }
    int internalHandler(struct SCrashContext* context) SKR_NOEXCEPT;

private:
    skr::String app_name;
    skr::String dump_file;
    HANDLE      dbghelp_dll = nullptr;
    bool        initialized = false;

private:
    static DWORD WINAPI StackOverflowThreadFunction(LPVOID lpParameter) SKR_NOEXCEPT;
    static LONG WINAPI  SehHandler(PEXCEPTION_POINTERS pExceptionPtrs) SKR_NOEXCEPT;
    // C++ terminate handler
    static void WINAPI TerminateHandler() SKR_NOEXCEPT;
    // C++ unexpected handler
    static void WINAPI UnexpectedHandler() SKR_NOEXCEPT;
#if _MSC_VER >= 1300
    static int WINAPI  NewHandler(size_t sz) SKR_NOEXCEPT;
    static void WINAPI PureCallHandler() SKR_NOEXCEPT;
#endif
#if _MSC_VER >= 1400
    static void WINAPI InvalidParameterHandler(const wchar_t* expression, const wchar_t* function,
                                               const wchar_t* file, unsigned int line, uintptr_t pReserved) SKR_NOEXCEPT;
#endif

    LPTOP_LEVEL_EXCEPTION_FILTER prevExceptionFilter = nullptr;
    terminate_handler            prevTerm            = nullptr;
    unexpected_handler           prevUnexp           = nullptr;
#if _MSC_VER >= 1300
    _purecall_handler prevPurec      = nullptr;
    _PNH              prevNewHandler = nullptr;
#endif
#if _MSC_VER >= 1400
    _invalid_parameter_handler prevInvpar = nullptr;
#endif
};

WinCrashHandler windows_crash_handler;

WinCrashHandler::~WinCrashHandler() SKR_NOEXCEPT
{
}

bool WinCrashHandler::Initialize() SKR_NOEXCEPT
{
    crashSetErrorMsg(u8"Unspecified error.");
    app_name = skr_get_current_process_name();

    // load dbghelp.dll
    dbghelp_dll = ::LoadLibraryA((const char*)kDebugHelpDLLName);
    if (!dbghelp_dll)
    {
        crashSetErrorMsg(u8"Failed to load dbghelp.dll.");
        return false;
    }

    if (!SCrashHandler::Initialize())
    {
        return false;
    }

    // The following code is intended to fix the issue with 32-bit applications in 64-bit environment.
    // http://support.microsoft.com/kb/976038/en-us
    // http://code.google.com/p/crashrpt/issues/detail?id=104
    typedef BOOL(WINAPI * SETPROCESSUSERMODEEXCEPTIONPOLICY)(DWORD dwFlags);
    typedef BOOL(WINAPI * GETPROCESSUSERMODEEXCEPTIONPOLICY)(LPDWORD lpFlags);
#define PROCESS_CALLBACK_FILTER_ENABLED 0x1
    HMODULE hKernel32 = LoadLibraryA("kernel32.dll");
    if (hKernel32 != NULL)
    {
        SETPROCESSUSERMODEEXCEPTIONPOLICY pfnSetProcessUserModeExceptionPolicy =
        (SETPROCESSUSERMODEEXCEPTIONPOLICY)GetProcAddress(hKernel32, "SetProcessUserModeExceptionPolicy");
        GETPROCESSUSERMODEEXCEPTIONPOLICY pfnGetProcessUserModeExceptionPolicy =
        (GETPROCESSUSERMODEEXCEPTIONPOLICY)GetProcAddress(hKernel32, "GetProcessUserModeExceptionPolicy");
        if (pfnSetProcessUserModeExceptionPolicy != NULL &&
            pfnGetProcessUserModeExceptionPolicy != NULL)
        {
            DWORD dwFlags = 0;
            if (pfnGetProcessUserModeExceptionPolicy(&dwFlags))
            {
                pfnSetProcessUserModeExceptionPolicy(dwFlags & ~PROCESS_CALLBACK_FILTER_ENABLED);
            }
        }
        FreeLibrary(hKernel32);
    }

    // TODO: deadlock detection
    // we can create another process that monitors deadlocks in current process.

    crashSetErrorMsg(u8"Sucessfully initialized crash handler.");
    initialized = true;

    skr_crash_handler_add_callback(this, +[](struct SCrashContext* context, void* usr_data) -> int {
        auto this_ = reinterpret_cast<WinCrashHandler*>(usr_data);
        return this_->internalHandler(context); }, this);

    return initialized;
}

int WinCrashHandler::internalHandler(struct SCrashContext* context) SKR_NOEXCEPT
{
    debug_break();
    if (!context->exception_pointers)
        context->exception_pointers = (PEXCEPTION_POINTERS)_pxcptinfoptrs;

    const auto  type   = MB_ABORTRETRYIGNORE | MB_ICONERROR;
    const auto  reason = context->reason;
    skr::String why    = skr::format(
    u8"Crashed! Reason: {}",
    skr_crash_code_string(reason));

    SKR_LOG_FATAL(why.c_str());

    // save crash minidump
    {
        char8_t    currentPath[1024];
        SYSTEMTIME localTime;
        ::GetCurrentDirectoryA(MAX_PATH, (char*)currentPath);
        ::GetLocalTime(&localTime);

        skr::String dateTime = skr::format(u8"{}-{}-{}-{}-{}-{}-{}",
                                           localTime.wYear, localTime.wMonth, localTime.wDay, localTime.wHour,
                                           localTime.wMinute, localTime.wSecond, localTime.wMilliseconds);

        skr::String dumpPath  = !dump_file.is_empty() ? dump_file : skr::format(u8"{}\\{}-minidump-{}.dmp", currentPath, skr_get_current_process_name(), dateTime);
        auto length = dumpPath.to_u16_length();
        std::wstring dumpPathW; 
        dumpPathW.resize(length);
        dumpPath.to_u16((skr_char16*)dumpPathW.data());
        HANDLE lhDumpFile = ::CreateFileW(dumpPathW.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        MINIDUMP_EXCEPTION_INFORMATION loExceptionInfo;
        loExceptionInfo.ExceptionPointers = context->exception_pointers;
        loExceptionInfo.ThreadId          = ::GetCurrentThreadId();
        loExceptionInfo.ClientPointers    = TRUE;

        ::MiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(), lhDumpFile, MiniDumpNormal, &loExceptionInfo, NULL, NULL);

        ::CloseHandle(lhDumpFile);
    }

    // show message box
    ::MessageBoxExA(nullptr,
                    why.c_str_raw(),
                    "Crash 了！",
                    type, 0);

    return 0;
}

bool WinCrashHandler::Finalize() SKR_NOEXCEPT
{
    return SCrashHandler::Finalize();
}

DWORD WINAPI WinCrashHandler::StackOverflowThreadFunction(LPVOID lpParameter) SKR_NOEXCEPT
{
    auto&               this_          = windows_crash_handler;
    PEXCEPTION_POINTERS pExceptionPtrs = reinterpret_cast<PEXCEPTION_POINTERS>(lpParameter);
    const auto          reason         = kCrashCodeStackOverflow;
    this_.handleFunction([&]() {
        this_.visit_callbacks([&](const CallbackWrapper& wrapper) {
            auto ctx                = this_.getCrashContext(reason);
            ctx->exception_pointers = pExceptionPtrs;
            wrapper.callback(ctx, wrapper.usr_data);
        });
    },
                         reason);
    return 0;
}

void WINAPI WinCrashHandler::TerminateHandler() SKR_NOEXCEPT
{
    auto&      this_  = windows_crash_handler;
    const auto reason = kCrashCodeTerminate;
    this_.handleFunction([&]() {
        this_.visit_callbacks([&](const CallbackWrapper& wrapper) {
            auto ctx = this_.getCrashContext(reason);
            wrapper.callback(ctx, wrapper.usr_data);
        });
    },
                         reason);
}

void WINAPI WinCrashHandler::UnexpectedHandler() SKR_NOEXCEPT
{
    auto&      this_  = windows_crash_handler;
    const auto reason = kCrashCodeUnexpected;
    this_.handleFunction([&]() {
        this_.visit_callbacks([&](const CallbackWrapper& wrapper) {
            auto ctx = this_.getCrashContext(reason);
            wrapper.callback(ctx, wrapper.usr_data);
        });
    },
                         reason);
}

LONG WINAPI WinCrashHandler::SehHandler(PEXCEPTION_POINTERS pExceptionPtrs) SKR_NOEXCEPT
{
    auto& this_ = windows_crash_handler;
    if (!this_.initialized)
    {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    // Handle stack overflow in a separate thread.
    // Vojtech: Based on martin.bis...@gmail.com comment in
    //	http://groups.google.com/group/crashrpt/browse_thread/thread/a1dbcc56acb58b27/fbd0151dd8e26daf?lnk=gst&q=stack+overflow#fbd0151dd8e26daf
    const auto hasExceptionRecord = pExceptionPtrs && pExceptionPtrs->ExceptionRecord;
    const auto isStackOverflow    = hasExceptionRecord && pExceptionPtrs->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW;
    if (isStackOverflow)
    {
        // Special case to handle the stack overflow exception.
        // The dump will be realized from another thread.
        // Create another thread that will do the dump.
        HANDLE thread = ::CreateThread(0, 0,
                                       &StackOverflowThreadFunction, pExceptionPtrs, 0, 0);
        ::WaitForSingleObject(thread, INFINITE);
        ::CloseHandle(thread);

        const auto reason = kCrashCodeUnhandled;
        this_.handleFunction([&]() {
            this_.visit_callbacks([&](const CallbackWrapper& wrapper) {
                auto ctx                = this_.getCrashContext(reason);
                ctx->exception_pointers = pExceptionPtrs;
                wrapper.callback(ctx, wrapper.usr_data);
            });
        },
                             reason);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

#if _MSC_VER >= 1300
void WINAPI WinCrashHandler::PureCallHandler() SKR_NOEXCEPT
{
    auto&      this_  = windows_crash_handler;
    const auto reason = kCrashCodePureVirtual;
    this_.handleFunction([&]() {
        this_.visit_callbacks([&](const CallbackWrapper& wrapper) {
            auto ctx = this_.getCrashContext(reason);
            wrapper.callback(ctx, wrapper.usr_data);
        });
    }, reason);
}

int WINAPI WinCrashHandler::NewHandler(size_t sz) SKR_NOEXCEPT
{
    auto&      this_  = windows_crash_handler;
    const auto reason = kCrashCodeOpNewError;
    this_.handleFunction([&]() {
        this_.visit_callbacks([&](const CallbackWrapper& wrapper) {
            auto ctx = this_.getCrashContext(reason);
            wrapper.callback(ctx, wrapper.usr_data);
        });
    }, reason);
    return 0;
}
#endif

#if _MSC_VER >= 1400
void WINAPI WinCrashHandler::InvalidParameterHandler(const wchar_t* expression, const wchar_t* function,
                                                     const wchar_t* file, unsigned int line, uintptr_t pReserved) SKR_NOEXCEPT
{
    (void)pReserved;
    auto&      this_  = windows_crash_handler;
    const auto reason = kCrashCodeInvalidParam;
    this_.handleFunction([&]() {
        this_.visit_callbacks([&](const CallbackWrapper& wrapper) {
            auto ctx = this_.getCrashContext(reason);
            wrapper.callback(ctx, wrapper.usr_data);
        });
    }, reason);
}
#endif

bool WinCrashHandler::SetProcessSignalHandlers() SKR_NOEXCEPT
{
    prevExceptionFilter = ::SetUnhandledExceptionFilter(&SehHandler);

    _set_error_mode(_OUT_TO_STDERR);

#if _MSC_VER >= 1300
    // Catch pure virtual function calls.
    // Because there is one _purecall_handler for the whole process,
    // calling this function immediately impacts all threads. The last caller on any thread sets the handler.
    // http://msdn.microsoft.com/en-us/library/t296ys27.aspx
    prevPurec = _set_purecall_handler(PureCallHandler);

    // Catch new operator memory allocation exceptions
    _set_new_mode(1); // Force malloc() to call new handler too
    prevNewHandler = _set_new_handler(NewHandler);
#endif

#if _MSC_VER >= 1400
    // Catch invalid parameter exceptions.
    prevInvpar = _set_invalid_parameter_handler(InvalidParameterHandler);

    _set_abort_behavior(_CALL_REPORTFAULT, _CALL_REPORTFAULT);
#endif

    return SCrashHandler::SetProcessSignalHandlers();
}

bool WinCrashHandler::SetThreadSignalHandlers() SKR_NOEXCEPT
{
    prevTerm  = set_terminate(TerminateHandler);
    prevUnexp = set_unexpected(UnexpectedHandler);
    return SCrashHandler::SetThreadSignalHandlers();
}

bool WinCrashHandler::UnsetProcessSignalHandlers() SKR_NOEXCEPT
{
    if (prevExceptionFilter)
        ::SetUnhandledExceptionFilter(prevExceptionFilter);

#if _MSC_VER >= 1300
    if (prevPurec)
        _set_purecall_handler(prevPurec);
    if (prevNewHandler)
        _set_new_handler(prevNewHandler);
#endif

#if _MSC_VER >= 1400
    if (prevInvpar)
        _set_invalid_parameter_handler(prevInvpar);
#endif

    return SCrashHandler::UnsetProcessSignalHandlers();
}

bool WinCrashHandler::UnsetThreadSignalHandlers() SKR_NOEXCEPT
{
    if (prevTerm != NULL)
        set_terminate(prevTerm);

    if (prevUnexp != NULL)
        set_unexpected(prevUnexp);

    return SCrashHandler::UnsetThreadSignalHandlers();
}
} // namespace

extern "C" {

SKR_CORE_API SCrashHandlerId skr_initialize_crash_handler() SKR_NOEXCEPT
{
    auto& this_ = ::windows_crash_handler;
    this_.Initialize();
    return &this_;
}

SKR_CORE_API void skr_crash_handler_set_dump_file(const char8_t* dump_file) SKR_NOEXCEPT
{
    auto& this_ = ::windows_crash_handler;
    this_.SetDumpFile(dump_file);
}

SKR_CORE_API SCrashHandlerId skr_crash_handler_get() SKR_NOEXCEPT
{
    return &::windows_crash_handler;
}

SKR_CORE_API void skr_finalize_crash_handler() SKR_NOEXCEPT
{
    auto& this_ = ::windows_crash_handler;
    this_.Finalize();
}
}