#define _GNU_SOURCE
#include <unistd.h>
#include <spawn.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <signal.h>

#include "SkrBase/misc/debug.h"
#include "SkrCore/memory/memory.h"
#include "SkrCore/process.h"
#include "SkrCore/log.h"

#include "SkrContainersDef/string.hpp"
#include "SkrContainersDef/vector.hpp"

typedef struct SProcess {
    pid_t pid;
} SProcess;

SProcessHandle skr_run_process(const char8_t* command, const char8_t** arguments, uint32_t arg_count, const char8_t* stdout_file)
{
    skr::Vector<skr::String> Args;
    for (size_t i = 0; i < arg_count; ++i)
    {
        Args.add(arguments[i]);
    }
    char*      Argv[256] = { NULL };
    const auto Argc      = Args.size();
    for (size_t i = 0; i < Argc; ++i)
    {
        Argv[i] = (char*)Args[i].c_str();
    }

    // int errcode = system(cmd.c_str()); (void)errcode;
    extern char** environ; // provided by libc
    pid_t         ChildPid = -1;

    posix_spawnattr_t SpawnAttr;
    posix_spawnattr_init(&SpawnAttr);
    short int SpawnFlags = 0;

    // unmask all signals and set realtime signals to default for children
    // the latter is particularly important for mono, which otherwise will crash attempting to find usable signals
    // (NOTE: setting all signals to default fails)
    sigset_t EmptySignalSet;
    sigemptyset(&EmptySignalSet);
    posix_spawnattr_setsigmask(&SpawnAttr, &EmptySignalSet);
    SpawnFlags |= POSIX_SPAWN_SETSIGMASK;

    sigset_t SetToDefaultSignalSet;
    sigemptyset(&SetToDefaultSignalSet);

    long SigrtMin, SigrtMax;
#if defined(SIGRTMIN) && defined(SIGRTMAX)
    SigrtMin = SIGRTMIN;
    SigrtMax = SIGRTMAX;
#elif defined(_SC_SIGRT_MIN) && defined(_SC_SIGRT_MAX)
    SigrtMin = sysconf(_SC_SIGRT_MIN);
    SigrtMax = sysconf(_SC_SIGRT_MAX);
#else
    SigrtMin = SIGUSR1;
    SigrtMax = SIGUSR2;
#endif
    for (int SigNum = SigrtMin; SigNum <= SigrtMax; ++SigNum)
    {
        sigaddset(&SetToDefaultSignalSet, SigNum);
    }

    posix_spawnattr_setsigdefault(&SpawnAttr, &SetToDefaultSignalSet);
    SpawnFlags |= POSIX_SPAWN_SETSIGDEF;

    // Makes spawned processes have its own unique group id so we can kill the entire group with out killing the parent
    SpawnFlags |= POSIX_SPAWN_SETPGROUP;

    int PosixSpawnErrNo = -1;
    {
#ifdef POSIX_SPAWN_USEVFORK
        // if we don't have any actions to do, use a faster route that will use vfork() instead.
        // This is not just faster, it is crucial when spawning a crash reporter to report a crash due to stack overflow in a thread
        // since otherwise atfork handlers will get called and posix_spawn() will crash (in glibc's __reclaim_stacks()).
        // However, it has its problems, see:
        //		http://ewontfix.com/7/
        //		https://sourceware.org/bugzilla/show_bug.cgi?id=14750
        //		https://sourceware.org/bugzilla/show_bug.cgi?id=14749
        SpawnFlags |= POSIX_SPAWN_USEVFORK;
#endif

        posix_spawnattr_setflags(&SpawnAttr, SpawnFlags);
        PosixSpawnErrNo = posix_spawn(&ChildPid, (const char*)command, nullptr, &SpawnAttr, Argv, environ);
    }
    posix_spawnattr_destroy(&SpawnAttr);

    if (PosixSpawnErrNo != 0)
    {
        SKR_LOG_FATAL(u8"skr_run_process: posix_spawn() failed (%d, %s)", PosixSpawnErrNo, strerror(PosixSpawnErrNo));
        return nullptr;
    }

    SProcessHandle result = SkrNew<SProcess>();
    result->pid           = ChildPid;
    return result;
}

const char8_t* skr_get_current_process_name()
{
#if defined(__APPLE__) || defined(__FreeBSD__)
    return (const char8_t*)getprogname();
#elif defined(_GNU_SOURCE)
    return (const char8_t*)program_invocation_name;
#elif defined(_WIN32)
    return (const char8_t*)__argv[0];
#else
    return u8"?";
#endif
}

SProcessId skr_get_current_process_id()
{
    return getpid();
}

int skr_wait_process(SProcessHandle process)
{
    const auto pid = process->pid;
    int        status;
    if (waitpid(pid, &status, WNOHANG) == -1)
    {
        perror("wait() error");
    }
    else
    {
        if (WIFEXITED(status))
            return 0;
        return -1;
    }
    return -1;
}

SProcessId skr_get_process_id(SProcessHandle process)
{
    return process->pid;
}