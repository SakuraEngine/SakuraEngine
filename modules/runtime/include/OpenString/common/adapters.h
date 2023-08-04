#pragma once
#include <cstddef>
#include "OpenString/common/definitions.h"
#include "SkrRT/platform/memory.h"

namespace ostr {

extern OPEN_STRING_API const ochar8_t* kOpenStringMemory;
template<class T>
struct allocator
{
	static T* allocate_single()
	{
		return (T*)sakura_new_nN(1, sizeof(T), (const char*)kOpenStringMemory);
	}
	
	static T* allocate_array(const size_t count)
	{
		return (T*)sakura_new_nN(count, sizeof(T), (const char*)kOpenStringMemory);
	}

	static void deallocate_single(const T* ptr)
	{
		sakura_freeN((void*)ptr, (const char*)kOpenStringMemory);
	}

	static void deallocate_array(const T* ptr)
	{
		sakura_freeN((void*)ptr, (const char*)kOpenStringMemory);
	}
	
	template<class...Args>
	static void placement_construct(void* ptr, Args&&...args) noexcept
	{
		new(ptr) T{ std::forward<Args>(args)... };
	}
};

}