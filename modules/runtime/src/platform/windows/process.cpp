#include "../../pch.hpp"
#include "SkrRT/platform/process.h"
#include "SkrRT/platform/memory.h"
#include "SkrRT/platform/atomic.h"

typedef struct SProcess
{
	PROCESS_INFORMATION processInfo;
	HANDLE stdOut = NULL;
} SProcess;

SProcessHandle skr_run_process(const char8_t* command, const char8_t** arguments, uint32_t arg_count, const char8_t* stdout_file)
{
    skr::string commandLine = skr::format(u8"\"{}\"", command); 
	for (size_t i = 0; i < arg_count; ++i)
	{
		commandLine += u8" ";
		commandLine += skr::string(arguments[i]);
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
	const char8_t* v = vs ? (const char8_t*)vs[0] : nullptr;
	if (!v)
	{
		static char pname[64] = { 0 };
		v = (const char8_t*)pname;

		enum
		{
			kNotInit = -1,
			kInitializing = 0,
			kInitialized = 1,
		};

		static SAtomic32 once = -1;
		if (skr_atomic32_cas_relaxed(&once, kNotInit, kInitializing) == kNotInit)
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
				DWORD buffSize = 1024;
				CHAR buffer[1024];
#if _WIN32_WINNT >= 0x0600
				if (QueryFullProcessImageNameA(handle, 0, buffer, &buffSize))
#else
				if (GetModuleFileNameExA(handle, NULL, buffer, buffSize))
#endif
				{
					// resolve to relative
					const auto p = PathFindFileNameA(buffer); // remove path
					const auto l = strlen(p);
					const auto ll = (l > 63) ? 63 : l;
					memcpy(pname, p, ll);
					pname[ll] = '\0';
				}
				else
					printf("Error GetModuleBaseNameA : %lu", GetLastError());
				CloseHandle(handle);
			}
			else
			{
				printf("Error OpenProcess : %lu", GetLastError());
			}
			skr_atomic32_store_relaxed(&once, kInitialized);
		}
		while (skr_atomic32_load_relaxed(&once) != kInitialized) {}
	}
	return v ? v : u8"unknown";
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
