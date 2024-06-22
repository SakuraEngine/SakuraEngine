#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h> 
#include <shlwapi.h>
#pragma comment(lib, "Shlwapi")

#include "SkrCore/process.h"
#include "SkrCore/memory/memory.h"
#include "SkrContainers/string.hpp"
#include "SkrBase/atomic/atomic.h"

typedef struct SProcess
{
	PROCESS_INFORMATION processInfo;
	HANDLE stdOut = NULL;
} SProcess;

SProcessHandle skr_run_process(const char8_t* command, const char8_t** arguments, uint32_t arg_count, const char8_t* stdout_file)
{
    skr::String commandLine = skr::format(u8"\"{}\"", command); 
	for (size_t i = 0; i < arg_count; ++i)
	{
		commandLine += u8" ";
		commandLine += skr::String(arguments[i]);
	}	

	HANDLE stdOut = NULL;
	if (stdout_file)
	{
		SECURITY_ATTRIBUTES sa;
		sa.nLength = sizeof(sa);
		sa.lpSecurityDescriptor = NULL;
		sa.bInheritHandle = TRUE;

		size_t   pathLength = strlen((const char*)stdout_file) + 1;
		wchar_t* buffer = (wchar_t*)alloca(pathLength * sizeof(wchar_t));
		MultiByteToWideChar(CP_UTF8, 0, (const char*)stdout_file, (int)pathLength, buffer, (int)pathLength);
		
		stdOut = CreateFileW(buffer, FILE_APPEND_DATA, 
			FILE_SHARE_WRITE | FILE_SHARE_READ, &sa, CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL, NULL);
	}

	STARTUPINFOA startupInfo;
	PROCESS_INFORMATION processInfo;
	memset(&startupInfo, 0, sizeof startupInfo);
	memset(&processInfo, 0, sizeof processInfo);
	startupInfo.cb = sizeof(STARTUPINFO);
	// startupInfo.dwFlags |= STARTF_USESHOWWINDOW;
	if (stdOut)
	{
		startupInfo.dwFlags |= STARTF_USESTDHANDLES;
		startupInfo.hStdOutput = stdOut;
		startupInfo.hStdError = stdOut;
	}

	if (!CreateProcessA(
			NULL, (LPSTR)commandLine.c_str(), NULL,
			NULL, stdOut ? TRUE : FALSE, 0/*CREATE_NO_WINDOW*/, 
			NULL, NULL, &startupInfo, &processInfo))
		return nullptr;

	SProcessHandle result = SkrNew<SProcess>();
	result->processInfo = processInfo;
	result->stdOut = stdOut;
	return result;
}

const char8_t* skr_get_current_process_name()
{
	const auto vs = __argv;
	const char8_t* fullpath = vs ? (const char8_t*)vs[0] : nullptr;
	const char8_t* pname = nullptr;
	DWORD fullpath_size_ = 1024;
	static char fullpath_[1024] = { 0 };
	static char pname_[64] = { 0 };

	fullpath = (const char8_t*)fullpath_;
	pname = (const char8_t*)pname_;

	enum
	{
		kNotInit = -1,
		kInitializing = 0,
		kInitialized = 1,
	};

	static SAtomic32 once = kNotInit;
	int32_t init = kNotInit;
	if (atomic_compare_exchange_strong(&once, &init, (int32_t)kInitializing))
	{
		HANDLE handle = OpenProcess(
#if _WIN32_WINNT >= 0x0600
			PROCESS_QUERY_LIMITED_INFORMATION,
#else
			PROCESS_QUERY_INFORMATION | PROCESS_VM_READ
#endif
			FALSE,
			GetCurrentProcessId()
		);
		if (handle)
		{
#if _WIN32_WINNT >= 0x0600
			if (QueryFullProcessImageNameA(handle, 0, fullpath_, &fullpath_size_))
#else
			if (GetModuleFileNameExA(handle, NULL, fullpath_, fullpath_size_))
#endif
			{
				// resolve to relative
				const auto p = PathFindFileNameA((const char*)fullpath); // remove path
				const auto l = strlen(p);
				const auto ll = (l > 63) ? 63 : l;
				memcpy(pname_, p, ll);
				pname_[ll] = '\0';
			}
			else
				printf("Error GetModuleBaseNameA : %lu", GetLastError());
			CloseHandle(handle);
		}
		else
		{
			printf("Error OpenProcess : %lu", GetLastError());
		}
		atomic_store_relaxed(&once, kInitialized);
	}
	while (atomic_load_relaxed(&once) != kInitialized) {}
	return pname ? pname : u8"unknown";
}

SProcessId skr_get_current_process_id()
{
	return (SProcessId)GetCurrentProcessId();
}

SProcessId skr_get_process_id(SProcessHandle process)
{
	return (SProcessId)process->processInfo.dwProcessId;
}

int skr_wait_process(SProcessHandle process)
{
	WaitForSingleObject(process->processInfo.hProcess, INFINITE);
	DWORD exitCode;
	GetExitCodeProcess(process->processInfo.hProcess, &exitCode);

	CloseHandle(process->processInfo.hProcess);
	CloseHandle(process->processInfo.hThread);

	if (process->stdOut)
	{
		CloseHandle(process->stdOut);
	}
	SkrDelete(process);
	return exitCode;
}
