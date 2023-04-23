/////////////////////////////////////////////////////////////////////////////
// Copyright (c) Electronic Arts Inc. All rights reserved.
/////////////////////////////////////////////////////////////////////////////


#pragma once
#include "internal/config.h"

namespace eastl
{
	///////////////////////////////////////////////////////////////////////////////
	// allocator_sakura
	//
	// Implements an EASTL allocator that uses malloc/free as opposed to
	// new/delete or PPMalloc Malloc/Free.
	//
	// Example usage:
	//      vector<int, allocator_sakura> intVector;
	//
	class EASTL_API allocator_sakura
	{
	public:
		inline allocator_sakura(const char* = NULL) {}

		inline allocator_sakura(const allocator_sakura&) {}

		inline allocator_sakura(const allocator_sakura&, const char*) {}

		inline allocator_sakura& operator=(const allocator_sakura&) { return *this; }

		bool operator==(const allocator_sakura&) { return true; }

		bool operator!=(const allocator_sakura&) { return false; }

		void* allocate(size_t n, int /*flags*/ = 0);

		void* allocate(size_t n, size_t alignment, size_t alignmentOffset, int /*flags*/ = 0);

		void deallocate(void* p, size_t /*n*/);

		const char* get_name() const { return "allocator_sakura"; }

		void set_name(const char*) {}
	};
	inline bool operator==(const allocator_sakura&, const allocator_sakura&) { return true; }
	inline bool operator!=(const allocator_sakura&, const allocator_sakura&) { return false; }

	EASTL_API allocator_sakura* GetDefaultAllocatorSakura();
	EASTL_API allocator_sakura* SetDefaultAllocatorSakura(allocator_sakura* pAllocator);

} // namespace eastl
