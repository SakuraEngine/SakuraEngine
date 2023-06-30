#include <unistd.h> 
#include <spawn.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <asm/ioctls.h>
#include <sys/prctl.h>

#include "platform/debug.h"
#include "platform/memory.h"
#include "platform/process.h"

#include "containers/string.hpp"
#include <EASTL/vector.h>

typedef struct SProcess
{
    pid_t pid;
} SProcess;

SProcessHandle skr_run_process(const char8_t* command, const char8_t** arguments, uint32_t arg_count, const char8_t* stdout_file)
{
	eastl::vector<const char8_t*> argPtrs;
	skr::string Args;
	for (size_t i = 0; i < arg_count; ++i)
	{
		Args += skr::string(arguments[i]);
        Args += u8" ";
	}

	// int errcode = system(cmd.c_str()); (void)errcode;
	extern char ** environ;	// provided by libc
	pid_t ChildPid = -1;

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
	for (int SigNum = SIGRTMIN; SigNum <= SIGRTMAX; ++SigNum)
	{
		sigaddset(&SetToDefaultSignalSet, SigNum);
	}
	posix_spawnattr_setsigdefault(&SpawnAttr, &SetToDefaultSignalSet);
	SpawnFlags |= POSIX_SPAWN_SETSIGDEF;

	// Makes spawned processes have its own unique group id so we can kill the entire group with out killing the parent
	SpawnFlags |= POSIX_SPAWN_SETPGROUP;

	int PosixSpawnErrNo = -1;
    {
		// if we don't have any actions to do, use a faster route that will use vfork() instead.
		// This is not just faster, it is crucial when spawning a crash reporter to report a crash due to stack overflow in a thread
		// since otherwise atfork handlers will get called and posix_spawn() will crash (in glibc's __reclaim_stacks()).
		// However, it has its problems, see:
		//		http://ewontfix.com/7/
		//		https://sourceware.org/bugzilla/show_bug.cgi?id=14750
		//		https://sourceware.org/bugzilla/show_bug.cgi?id=14749
		SpawnFlags |= POSIX_SPAWN_USEVFORK;

		posix_spawnattr_setflags(&SpawnAttr, SpawnFlags);
		PosixSpawnErrNo = posix_spawn(&ChildPid, command, nullptr, &SpawnAttr, Args.c_str(), environ);
	}
	posix_spawnattr_destroy(&SpawnAttr);
    
    SProcessHandle result = SkrNew<SProcess>();
    result->pid = ChildPid;
    return result;
}

SProcessId skr_get_current_process_id()
{
    return getpid();
}

int skr_wait_process(SProcessHandle process)
{
    const auto pid = process->pid;
    int status;
    if ((pid = waitpid(pid, &status, WNOHANG)) == -1)
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