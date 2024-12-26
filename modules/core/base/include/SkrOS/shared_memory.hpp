#pragma once
#include "SkrBase/types/guid.h"

namespace skr
{

struct SharedMemory
{
public:
	SharedMemory(skr::GUID key, const unsigned int size, const unsigned int accessPermission = 0666);
	~SharedMemory();
	void* address(uint64_t offset = 0);

private:
	int64_t shared_handle;
	void*    base_address;
};

}