#pragma once
#include "SkrBase/config.h"

namespace skr
{

struct SharedMemory
{
public:
	SharedMemory(const unsigned int key, const unsigned int size, const unsigned int accessPermission = 0666);
	~SharedMemory();
	void* address(uint64_t offset = 0);

private:
	int64_t shared_handle;
	void*    base_address;
};

}