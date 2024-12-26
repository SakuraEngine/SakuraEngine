#include "SkrBase/misc/debug.h"
#include "SkrOS/shared_memory.hpp"
#include "./../winheaders.h"
#include <process.h>
#include <sddl.h>

#define ERRNO GetLastError()

namespace skr
{
inline static char* GetErrorStr(int errorCode)
{
	static char buf[0x100];
	buf[0] = 0;
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		buf,
		0x100,
		NULL);
	return buf;
}

inline static SECURITY_ATTRIBUTES* CreateSecurityDescriptor(const unsigned int accessPermission)
{
	static SECURITY_ATTRIBUTES security = {0};
	if (0 == security.nLength)
	{
		security.nLength = sizeof(security);
		const char* sharessdl = "D:P(A;;GA;;;WD)";
		BOOL bRet = ConvertStringSecurityDescriptorToSecurityDescriptorA(sharessdl, SDDL_REVISION_1, &security.lpSecurityDescriptor, NULL);
		if (0 == bRet)
		{
			SKR_ASSERT(0 && "Create security descriptor failed\r\n");
		}
		//LocalFree(security.lpSecurityDescriptor);
		//security.lpSecurityDescriptor = NULL;
	}
	return &security;
}

SharedMemory::SharedMemory(const unsigned int key, const unsigned int size, const unsigned int accessPermission)
{
	char nameHead[0x100] = { 0 };
	sprintf(nameHead, "Global\\SharedMemory_%u", key);

	shared_handle = (int64_t)CreateFileMappingA(
		INVALID_HANDLE_VALUE,
		CreateSecurityDescriptor(accessPermission),
		PAGE_READWRITE,
		0,
		size,
		nameHead
	);

	int iRet = GetLastError();
	if (NULL == shared_handle && ERROR_ACCESS_DENIED == iRet)
	{
		sprintf(nameHead, "MessageQueue_%u", key);
		shared_handle = (int64_t)CreateFileMappingA(
			INVALID_HANDLE_VALUE,
			CreateSecurityDescriptor(accessPermission),
			PAGE_READWRITE,
			0,
			size,
			nameHead
		);
	}

	iRet = GetLastError();
	if (NULL == shared_handle)
	{
		// LOG_ERROR("name=%s, Create shared memory failed, key=%u, error=%s\r\n", nameHead, key, GetErrorStr(ERRNO));
		exit(ERRNO);
	}

	base_address = (void*)MapViewOfFile(
		(HANDLE)shared_handle,//共享内存的句柄
		FILE_MAP_ALL_ACCESS,//可读写许可
		0,
		0,
		size
	);
	if (NULL == base_address)
	{
		// LOG_ERROR("name=%s, Get shared memory point failed, key=%u, error=%s\r\n", nameHead, key, GetErrorStr(ERRNO));
		exit(ERRNO);
	}

	// LOG_DEBUG("name=%s, Open shared memory successful\r\n", nameHead);
	if (ERROR_ALREADY_EXISTS != iRet)
	{
		memset(base_address, 0, size);
		// LOG_DEBUG("name=%s, Init shared memory\r\n", nameHead);
    }
}

SharedMemory::~SharedMemory()
{
    // unmap file mapping
	if (base_address != nullptr)
		UnmapViewOfFile(base_address);

	if (shared_handle != 0)
		CloseHandle((HANDLE)shared_handle);
}

void* SharedMemory::address(uint64_t offset)
{
    return (void*)((uint64_t)base_address + offset);
}

}