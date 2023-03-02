// OpenString - adapters for cooperating with user's project
//
// Users should edit this file to match their project's needs
//
// Copyright (c) 2022 - present, [Hoshizora Ming]
// All rights reserved.

#pragma once
#include <cstddef>
#include "common/definitions.h"

OPEN_STRING_NS_BEGIN

template<class T>
struct allocator
{
	static T* allocate_single()
	{
		return new T;
	}
	
	static T* allocate_array(const size_t count)
	{
		return new T[count];
	}

	static void deallocate_single(const T* ptr)
	{
		delete ptr;
	}

	static void deallocate_array(const T* ptr)
	{
		delete[] ptr;
	}
};

OPEN_STRING_NS_END
