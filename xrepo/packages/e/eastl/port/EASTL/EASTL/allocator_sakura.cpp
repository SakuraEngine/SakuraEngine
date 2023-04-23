/* clang-format off */
#include "internal/config.h"
#include "allocator_sakura.h"
#include <stdlib.h>

#ifndef EASTL_IMPORT
#ifdef RUNTIME_ALL_STATIC 
#define EASTL_IMPORT extern "C"
#else
    #if defined(_MSC_VER)
        #define EASTL_IMPORT __declspec(dllimport) extern "C"
    #else
        #define EASTL_IMPORT __attribute__((visibility("default"))) extern "C"
    #endif
#endif
#endif

EASTL_IMPORT void* containers_malloc_aligned(size_t size, size_t alignment);
EASTL_IMPORT void containers_free_aligned(void* p, size_t alignment);

#define core_memalign containers_malloc_aligned
#define core_free_aligned containers_free_aligned

	namespace eastl
	{
		void* allocator_sakura::allocate(size_t n, int /*flags*/)
		{ 
			void* p = core_memalign(n, 1);
			return p;
		}

		void* allocator_sakura::allocate(size_t n, size_t alignment, size_t alignmentOffset, int /*flags*/)
		{
		    if ((alignmentOffset % alignment) == 0) // We check for (offset % alignmnent == 0) instead of (offset == 0) because any block which is
													// aligned on e.g. 64 also is aligned at an offset of 64 by definition.
			{
				void* p = core_memalign(n, alignment);
				return p;
			}

			return NULL;
		}

		void allocator_sakura::deallocate(void* p, size_t /*n*/)
		{ 
			core_free_aligned(p, 1);
		}

		/// gDefaultAllocator
		/// Default global allocator_sakura instance. 
		EASTL_API allocator_sakura  gDefaultAllocatorsakura;
		EASTL_API allocator_sakura* gpDefaultAllocatorsakura = &gDefaultAllocatorsakura;

		EASTL_API allocator_sakura* GetDefaultAllocatorSakura()
		{
			return gpDefaultAllocatorsakura;
		}

		EASTL_API allocator_sakura* SetDefaultAllocatorSakura(allocator_sakura* pAllocator)
		{
			allocator_sakura* const pPrevAllocator = gpDefaultAllocatorsakura;
			gpDefaultAllocatorsakura = pAllocator;
			return pPrevAllocator;
		}

	} // namespace eastl


/* clang-format on */