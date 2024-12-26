#include "SkrOS/shared_memory.hpp"

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>

namespace skr
{
SharedMemory::SharedMemory(const unsigned int key, const unsigned int size, const unsigned int accessPermission)
{
	if (0 == key)
	{
		// LOG_ERROR("key=%u, Create shared memory failed\r\n", key);
		exit(-1);
	}
	bool exist = false;
	//创建共享内存
	shared_handle = shmget((key_t)key, size, accessPermission | IPC_CREAT | IPC_EXCL);
	// LOG_TRACE("Shared memory create, error=%s\r\n", GetErrorStr(ERRNO));
	if (errno == EEXIST)
	{
		exist = true;
		// LOG_DEBUG("Shared memory already exists\r\n");
		shared_handle = shmget((key_t)key, size, accessPermission | IPC_CREAT);
	}
	// LOG_TRACE("shared_handle=%d\r\n", shared_handle);
	if (shared_handle == -1)
	{
		// LOG_ERROR("shmget failed, key=%u, error=%s\r\n", key, GetErrorStr(ERRNO));
		exit(ERRNO);
	}
	//将共享内存连接到当前进程的地址空间
	base_address = (void*)shmat(shared_handle, 0, 0);
	if (base_address == (void*)-1)
	{
		// LOG_ERROR("shmat failed, key=%u, error=%s\r\n", key, GetErrorStr(ERRNO));
		exit(ERRNO);
	}

	if (!exist)
	{
		memset(base_address, 0, size);
		// LOG_DEBUG("key=%u, Init shared memory\r\n", key);
	}
}

SharedMemory::~SharedMemory()
{
    // unmap file mapping
	if (base_address != nullptr)
		shmdt(base_address);

	if (-1 != shared_handle)
		shmctl(shared_handle, IPC_RMID, NULL);
}

void* SharedMemory::address(uint64_t offset)
{
    return (void*)((uint64_t)base_address + offset);
}

}