#pragma once
#include <cstddef>
#include "OpenString/common/definitions.h"
#include "SkrMemory/memory.h"

namespace skr
{
	extern SKR_CORE_API const ochar8_t* kOpenStringMemory;
}

namespace ostr {
template<class T>
struct allocator
{
	static T* allocate_single()
	{
		return (T*)sakura_new_nN(1, sizeof(T), (const char*)skr::kOpenStringMemory);
	}
	
	static T* allocate_array(const size_t count)
	{
		return (T*)sakura_new_nN(count, sizeof(T), (const char*)skr::kOpenStringMemory);
	}

	static void deallocate_single(const T* ptr)
	{
		sakura_freeN((void*)ptr, (const char*)skr::kOpenStringMemory);
	}

	static void deallocate_array(const T* ptr)
	{
		sakura_freeN((void*)ptr, (const char*)skr::kOpenStringMemory);
	}
	
	template<class...Args>
	static void placement_construct(void* ptr, Args&&...args) noexcept
	{
		new(ptr) T{ std::forward<Args>(args)... };
	}
};

}