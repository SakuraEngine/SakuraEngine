// OpenString - adapters for cooperating with user's project
//
// Users should edit this file to match their project's needs
//
// Copyright (c) 2022 - present, [Hoshizora Ming]
// All rights reserved.

#pragma once
#include <cstddef>
#include "common/definitions.h"
#include "SkrRT/platform/memory.h"

OPEN_STRING_NS_BEGIN

extern OPEN_STRING_API const char* kOpenStringMemory;
template<class T>
struct allocator
{
	static T* allocate_single()
	{
		return (T*)sakura_new_nN(1, sizeof(T), kOpenStringMemory);
	}
	
	static T* allocate_array(const size_t count)
	{
		return (T*)sakura_new_nN(count, sizeof(T), kOpenStringMemory);
	}

	static void deallocate_single(const T* ptr)
	{
		sakura_freeN((void*)ptr, kOpenStringMemory);
	}

	static void deallocate_array(const T* ptr)
	{
		sakura_freeN((void*)ptr, kOpenStringMemory);
	}
};

OPEN_STRING_NS_END
